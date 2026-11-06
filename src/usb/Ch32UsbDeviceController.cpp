#include "usb/Ch32UsbDeviceController.h"

#if defined(ART_CH32_HAS_USB_DEVICE_CONTROLLER)

#include <array>

namespace {

using art::usb::endpointNumber;
using art::usb::isInEndpoint;

constexpr uint8_t kEndpoint0In = 0x80u;
constexpr uint16_t kEndpoint0Bit = 0x01u;

// Index into the per-endpoint-direction arrays: OUT 0..15, IN 16..31.
size_t slot(uint8_t endpointAddress) {
   return endpointNumber(endpointAddress) + (isInEndpoint(endpointAddress) ? hw::UsbDevice::EndpointCount : 0u);
}

bool isPacketEvent(hw::usb::EventType type) {
   return type == hw::usb::EventType::Out || type == hw::usb::EventType::InComplete;
}

hw::usb::EndpointType toHardware(art::usb::EndpointType type) {
   switch(type) {
   case art::usb::EndpointType::Control:
      return hw::usb::EndpointType::Control;
   case art::usb::EndpointType::Isochronous:
      return hw::usb::EndpointType::Isochronous;
   case art::usb::EndpointType::Bulk:
      return hw::usb::EndpointType::Bulk;
   case art::usb::EndpointType::Interrupt:
      break;
   }
   return hw::usb::EndpointType::Interrupt;
}

// One more store after a load: only the interrupt callback writes these
// counters. hw::UsbDevice::armOut() can call it in the main loop, but with
// the USB interrupt masked, so two writers never run at the same time.
template <typename T>
void increment(std::atomic<T> &counter, std::memory_order order) {
   counter.store(static_cast<T>(counter.load(std::memory_order_relaxed) + 1u), order);
}

} // namespace

Ch32UsbDeviceController::Ch32UsbDeviceController(hw::UsbDevice &device, RingBufferBase<Event> &eventQueue)
   : device_(device)
   , eventQueue_(eventQueue) {
}

void Ch32UsbDeviceController::begin() {
   // The interrupt is not connected yet, so the consumer may clear the queue.
   eventQueue_.clear();
   resetsQueued_.store(0u, std::memory_order_relaxed);
   setupsQueued_.store(0u, std::memory_order_relaxed);
   resetsDelivered_ = 0u;
   setupsDelivered_ = 0u;
   pendingAddress_.store(kNoAddress, std::memory_order_relaxed);
   for(auto &counter : packetsQueued_) {
      counter.store(0u, std::memory_order_relaxed);
   }
   packetsDelivered_.fill(0u);
   staleEvents_.fill(0u);
   deferredArms_ = 0u;
   openIn_ = 0u;
   openOut_ = 0u;
   device_.begin(&Ch32UsbDeviceController::onInterruptEvent, this);
}

void Ch32UsbDeviceController::end() {
   device_.end();
}

void Ch32UsbDeviceController::poll() {
   Event event{};
   while(eventQueue_.try_pop(event)) {
      deliver(event);
   }
}

void Ch32UsbDeviceController::onInterruptEvent(void *context, const Event &event) {
   static_cast<Ch32UsbDeviceController *>(context)->handleInterruptEvent(event);
}

void Ch32UsbDeviceController::handleInterruptEvent(const Event &event) {
   switch(event.type) {
   case hw::usb::EventType::Reset:
   case hw::usb::EventType::Setup:
      // The host aborted a SET_ADDRESS whose status stage did not complete.
      pendingAddress_.store(kNoAddress, std::memory_order_relaxed);
      break;
   case hw::usb::EventType::InComplete:
      // The status stage of SET_ADDRESS is complete. The host uses the new
      // address 2 ms later, so this cannot wait for the main loop.
      if(event.endpoint == kEndpoint0In) {
         const uint8_t address = pendingAddress_.load(std::memory_order_relaxed);
         if(address != kNoAddress) {
            device_.setAddress(address);
            pendingAddress_.store(kNoAddress, std::memory_order_relaxed);
         }
      }
      break;
   default:
      break;
   }

   if(!eventQueue_.try_push(event)) {
      increment(droppedEvents_, std::memory_order_relaxed);
      return;
   }
   if(event.type == hw::usb::EventType::Reset) {
      increment(resetsQueued_, std::memory_order_release);
   } else if(event.type == hw::usb::EventType::Setup) {
      increment(setupsQueued_, std::memory_order_release);
   } else if(isPacketEvent(event.type)) {
      increment(packetsQueued_[slot(event.endpoint)], std::memory_order_release);
   }
}

void Ch32UsbDeviceController::deliver(const Event &event) {
   // FIFO order: while a reset waits in the queue, every event before it
   // belongs to the endpoints and transfers that the reset ended.
   const bool beforeReset = resetsDelivered_ != resetsQueued_.load(std::memory_order_acquire);

   if(event.type == hw::usb::EventType::Reset) {
      ++resetsDelivered_;
      // hw::UsbDevice has closed every endpoint and reopened EP0 for the
      // protocol layer to open again. A handler that ran during the reset
      // interrupt (for example SET_CONFIGURATION) can have opened endpoints
      // again in the hardware since. The protocol layer has not seen the
      // reset yet, so no endpoint after it is open: free them all.
      device_.closeAllEndpoints();
      openIn_ = 0u;
      openOut_ = 0u;
      deferredArms_ = 0u;
      if(handler_ != nullptr) {
         handler_->busReset();
      }
      return;
   }
   if(event.type == hw::usb::EventType::Setup) {
      ++setupsDelivered_;
   }
   if(isPacketEvent(event.type)) {
      const size_t index = slot(event.endpoint);
      ++packetsDelivered_[index];
      // Queued before closeEndpoint(): it belongs to the closed endpoint, not
      // to an endpoint that was opened again with the same address.
      if(staleEvents_[index] != 0u) {
         --staleEvents_[index];
         return;
      }
   }
   if(beforeReset || handler_ == nullptr) {
      return;
   }

   switch(event.type) {
   case hw::usb::EventType::Setup:
      handler_->setupReceived(art::usb::SetupPacket::fromBytes(event.setup));
      break;
   case hw::usb::EventType::Out:
      if(isOpen(event.endpoint)) {
         std::array<uint8_t, hw::UsbDevice::BulkMaxPacketSize> packet; // readOut() fills size bytes
         const uint16_t size = device_.readOut(event.endpoint, packet);
         handler_->outPacketReceived(event.endpoint, art::span<const uint8_t>(packet.data(), size));
         armDeferredOut(event.endpoint);
      }
      break;
   case hw::usb::EventType::InComplete:
      if(isOpen(event.endpoint)) {
         handler_->inPacketSent(event.endpoint);
      }
      break;
   case hw::usb::EventType::Suspend:
      handler_->suspended();
      break;
   case hw::usb::EventType::Resume:
      handler_->resumed();
      break;
   case hw::usb::EventType::Reset:
      break;
   }
}

void Ch32UsbDeviceController::setEventHandler(art::usb::ControllerEvents *handler) {
   handler_ = handler;
}

void Ch32UsbDeviceController::connect() {
   device_.connect();
}

void Ch32UsbDeviceController::disconnect() {
   device_.disconnect();
}

void Ch32UsbDeviceController::setAddress(uint8_t address) {
   pendingAddress_.store(address, std::memory_order_relaxed);
   // A SETUP or a reset that the interrupt queued before the store above
   // already aborted this SET_ADDRESS, and the interrupt could not drop the
   // address yet. One that comes after the store drops it in the interrupt.
   // This needs the store before the loads. An acquire load does not keep an
   // earlier relaxed store before it, so the fence does (one core, so a
   // compiler barrier is enough). Do not remove or relax it.
   std::atomic_signal_fence(std::memory_order_seq_cst);
   if(setupsQueued_.load(std::memory_order_acquire) != setupsDelivered_ ||
      resetsQueued_.load(std::memory_order_acquire) != resetsDelivered_) {
      pendingAddress_.store(kNoAddress, std::memory_order_relaxed);
   }
}

bool Ch32UsbDeviceController::openEndpoint(const art::usb::EndpointConfig &config) {
   const uint8_t number = endpointNumber(config.address);
   if(number == 0u) {
      // hw::UsbDevice opens EP0 itself on every bus reset. Opening it again
      // would reset its data toggles, and a SETUP packet can already be in.
      if(config.type != art::usb::EndpointType::Control || isInEndpoint(config.address) ||
         config.maxPacketSize != hw::UsbDevice::Ep0MaxPacketSize) {
         return false;
      }
      openIn_ |= kEndpoint0Bit;
      openOut_ |= kEndpoint0Bit;
      return true;
   }
   // The OUT packet is copied to a buffer of the bulk size, so a larger OUT packet (an interrupt endpoint of up
   // to 1024 bytes) would be cut.
   if(!isInEndpoint(config.address) && config.maxPacketSize > hw::UsbDevice::BulkMaxPacketSize) {
      return false;
   }
   const bool doubleBuffered = config.address == doubleBufferedOut_ && config.type == art::usb::EndpointType::Bulk;
   if(!device_.openEndpoint(config.address, toHardware(config.type), config.maxPacketSize, doubleBuffered)) {
      return false;
   }
   (isInEndpoint(config.address) ? openIn_ : openOut_) |= static_cast<uint16_t>(1u << number);
   return true;
}

void Ch32UsbDeviceController::closeEndpoint(uint8_t endpointAddress) {
   if(!isOpen(endpointAddress)) {
      return;
   }
   const uint8_t number = endpointNumber(endpointAddress);
   const size_t index = slot(endpointAddress);
   deferredArms_ &= static_cast<uint16_t>(~(1u << number));
   if(number == 0u) {
      staleEvents_[index] = static_cast<uint8_t>(packetsQueued_[index].load(std::memory_order_acquire) - packetsDelivered_[index]);
      openIn_ &= static_cast<uint16_t>(~kEndpoint0Bit);
      openOut_ &= static_cast<uint16_t>(~kEndpoint0Bit);
      return;
   }
   (isInEndpoint(endpointAddress) ? openIn_ : openOut_) &= static_cast<uint16_t>(~(1u << number));
   // NAK first, then count: the driver reports a completed transfer that the interrupt has not handled yet
   // inside setNak(), so its event is in the queue when setNak() returns, and it is stale too. The driver sets
   // DATA0 again when the endpoint opens again (SET_INTERFACE).
   device_.setNak(endpointAddress);
   staleEvents_[index] = static_cast<uint8_t>(packetsQueued_[index].load(std::memory_order_acquire) - packetsDelivered_[index]);
   if(((openIn_ | openOut_) & static_cast<uint16_t>(~kEndpoint0Bit)) == 0u) {
      device_.closeAllEndpoints();
   }
}

void Ch32UsbDeviceController::setEndpointStall(uint8_t endpointAddress, bool stall) {
   if(!isOpen(endpointAddress) || isSupersededControlCall(endpointAddress)) {
      return;
   }
   if(endpointNumber(endpointAddress) != 0u) {
      // Drop a pending startIn() or armOut() first. EP0 keeps the single write: the hardware NAKs it after each
      // SETUP and each packet, and the next SETUP ends a STALL.
      device_.setNak(endpointAddress);
   }
   device_.setStall(endpointAddress, stall);
}

bool Ch32UsbDeviceController::startInTransfer(uint8_t endpointAddress, art::span<const uint8_t> packet) {
   if(!isInEndpoint(endpointAddress) || !isOpen(endpointAddress) || isSupersededControlCall(endpointAddress)) {
      return false;
   }
   return device_.startIn(endpointAddress, packet);
}

bool Ch32UsbDeviceController::cancelInTransfer(uint8_t endpointAddress) {
   if(!isInEndpoint(endpointAddress) || endpointNumber(endpointAddress) == 0u || !isOpen(endpointAddress)) {
      return true;
   }
   const size_t index = slot(endpointAddress);
   // A NAK write would end the STALL. A stalled endpoint sends nothing, so only the queue decides.
   // The driver reports a packet that the hardware has completed inside setNak(), so its event is in the queue
   // when setNak() returns. A flip of the data toggle that the write undid is repaired by the driver.
   if(!device_.isStalled(endpointAddress)) {
      device_.setNak(endpointAddress);
   }
   // An InComplete event that waits in the queue belongs to a packet that
   // the host acknowledged: it is still delivered. Events that
   // closeEndpoint() made stale are dropped, so they do not count.
   const auto waiting = static_cast<uint8_t>(packetsQueued_[index].load(std::memory_order_acquire) - packetsDelivered_[index]);
   return waiting == staleEvents_[index];
}

void Ch32UsbDeviceController::armOutTransfer(uint8_t endpointAddress) {
   if(isInEndpoint(endpointAddress) || !isOpen(endpointAddress) || isSupersededControlCall(endpointAddress)) {
      return;
   }
   const uint8_t number = endpointNumber(endpointAddress);
   // An OUT packet of this endpoint that waits in the queue is still in packet
   // memory. The next packet would overwrite it, so arm only after poll() has
   // delivered it.
   if(number != 0u && isOutQueued(endpointAddress)) {
      deferredArms_ |= static_cast<uint16_t>(1u << number);
      return;
   }
   deferredArms_ &= static_cast<uint16_t>(~(1u << number));
   static_cast<void>(device_.armOut(endpointAddress));
}

bool Ch32UsbDeviceController::isOutQueued(uint8_t endpointAddress) const {
   const size_t index = slot(endpointAddress);
   return packetsQueued_[index].load(std::memory_order_acquire) != packetsDelivered_[index];
}

void Ch32UsbDeviceController::armDeferredOut(uint8_t endpointAddress) {
   const auto bit = static_cast<uint16_t>(1u << endpointNumber(endpointAddress));
   if((deferredArms_ & bit) != 0u && !isOutQueued(endpointAddress)) {
      deferredArms_ &= static_cast<uint16_t>(~bit);
      static_cast<void>(device_.armOut(endpointAddress));
   }
}

bool Ch32UsbDeviceController::isSupersededControlCall(uint8_t endpointAddress) const {
   // The hardware sets EP0 to NAK when a SETUP packet arrives. While that
   // SETUP (or a reset) waits in the queue, the protocol layer still works on
   // the transfer before it. Its data, status or STALL must not undo the NAK
   // and reach the new transfer. The handler of the newest SETUP is not
   // blocked: poll() counts a SETUP as delivered before it calls the handler.
   return endpointNumber(endpointAddress) == 0u && (setupsQueued_.load(std::memory_order_acquire) != setupsDelivered_ ||
                                                    resetsQueued_.load(std::memory_order_acquire) != resetsDelivered_);
}

bool Ch32UsbDeviceController::isOpen(uint8_t endpointAddress) const {
   const uint8_t number = endpointNumber(endpointAddress);
   if(number >= hw::UsbDevice::EndpointCount) {
      return false;
   }
   const uint16_t open = isInEndpoint(endpointAddress) ? openIn_ : openOut_;
   return (open & (1u << number)) != 0u;
}

#endif
