#pragma once

// The controller exists only where ch32v00x-system has a USB device driver
// for the family (CH32V30X: USBHS). On the others this header and its source
// compile to nothing.
#if __has_include(<UsbDevice.h>)
#include <UsbDevice.h>
#define ART_CH32_HAS_USB_DEVICE_CONTROLLER 1

#include <array>
#include <atomic>
#include <cstdint>
#include <type_traits>

#include <usb/DeviceController.h>
#include <utils/RingBuffer.h>
#include <utils/compat/span.h>

// startInTransfer() hands its art::span to hw::UsbDevice::startIn(), which
// takes a hw::span. Both must be the same type, or the call converts between
// two span types.
static_assert(std::is_same<art::span<const uint8_t>, hw::span<const uint8_t>>::value,
              "art::span (libArT) and hw::span (ch32v00x-system) are different types.");

/**
 * @brief libArT's usb::DeviceController on ch32v00x-system's hw::UsbDevice (USBHS of the CH32V30x).
 *
 * The counterpart of Stm32UsbDeviceController in libart-stm32, with the same threading model: the USB interrupt
 * only queues the events of hw::UsbDevice, and poll() gives them to the event handler (usb::Device) in the main
 * loop. The queue is a single-producer, single-consumer ring buffer. An OUT packet stays in the endpoint buffer
 * until poll() delivers it, because the endpoint NAKs until it is armed again.
 *
 * The same work runs in the interrupt as in the STM32 class: SET_ADDRESS is applied when the host acknowledges the
 * status stage on EP0 IN, a SETUP packet or a bus reset drops a stored address, and the controller ignores the
 * EP0 calls of an aborted transfer while a newer SETUP or a reset waits in the queue. poll() drops the packet
 * events that were queued before a bus reset or before closeEndpoint().
 *
 * Differences from the STM32 class:
 *  - No double-buffered OUT endpoint: hw::UsbDevice of the USBHS has none.
 *  - 16 endpoint numbers, and packets up to 512 bytes (bulk) or 1024 bytes (interrupt). The speed is the speed
 *    of the host: use speed() after the first bus reset to pick the descriptor set.
 *  - The D+ pull-up is internal (connect() and disconnect() only set a bit).
 *  - Endpoint response changes are single writes (hw::UsbDevice masks the USB interrupt), so closeEndpoint(),
 *    setEndpointStall() and cancelInTransfer() need no repeated NAK write.
 *
 * Port limits:
 *  - EP0 must have a bMaxPacketSize0 of 64 (hw::UsbDevice::Ep0MaxPacketSize).
 *  - cancelInTransfer() does not take back an EP0 packet. The control layer must not call it on EP0.
 *
 * Queue size: an endpoint NAKs until it is armed again, so each endpoint direction queues at most one Out or
 * InComplete event. Add SETUP packets and bus events. droppedEvents() counts the events that did not fit: the
 * newest event is dropped. A dropped event cannot be recovered without a new enumeration.
 *
 * Threading: call every function except the interrupt callback from the main loop, the same context that runs
 * poll().
 */
class Ch32UsbDeviceController final : public art::usb::DeviceController {
   public:
   using Event = hw::usb::Event;

   /**
    * @param device The USB peripheral driver.
    * @param eventQueue The queue between the interrupt and poll(). Used by this controller only.
    */
   Ch32UsbDeviceController(hw::UsbDevice &device, RingBufferBase<Event> &eventQueue);

   Ch32UsbDeviceController(const Ch32UsbDeviceController &) = delete;
   Ch32UsbDeviceController &operator=(const Ch32UsbDeviceController &) = delete;

   /// Clears the queue and starts the peripheral (hw::UsbDevice::begin()). The device stays disconnected until connect().
   void begin();

   /// Stops the peripheral (hw::UsbDevice::end()). The host sees a disconnect.
   void end();

   /// Gives the queued events to the event handler. Call it from the main loop.
   void poll();

   /// The speed that the host selected. Valid after the first bus reset.
   [[nodiscard]] hw::UsbDevice::Speed speed() const {
      return device_.speed();
   }

   /// The number of events that the interrupt could not queue because the queue was full.
   [[nodiscard]] uint32_t droppedEvents() const {
      return droppedEvents_.load(std::memory_order_relaxed);
   }

   // art::usb::DeviceController
   void setEventHandler(art::usb::ControllerEvents *handler) override;
   void connect() override;
   void disconnect() override;
   void setAddress(uint8_t address) override;
   bool openEndpoint(const art::usb::EndpointConfig &config) override;
   void closeEndpoint(uint8_t endpointAddress) override;
   void setEndpointStall(uint8_t endpointAddress, bool stall) override;
   bool startInTransfer(uint8_t endpointAddress, art::span<const uint8_t> packet) override;
   bool cancelInTransfer(uint8_t endpointAddress) override;
   void armOutTransfer(uint8_t endpointAddress) override;

   private:
   static constexpr uint8_t kNoAddress = 0xFFu;
   static constexpr size_t kSlots = 2u * hw::UsbDevice::EndpointCount;

   hw::UsbDevice &device_;
   RingBufferBase<Event> &eventQueue_;
   art::usb::ControllerEvents *handler_ = nullptr;

   // Open endpoints as the protocol layer sees them, one bit per endpoint number. Main loop only.
   uint16_t openIn_ = 0u;
   uint16_t openOut_ = 0u;

   // Shared with the interrupt. Only loads and stores (no atomic read-modify-write on the QingKe V4F). The
   // *Queued_ counters are written by the interrupt callback, the *Delivered_ counters by poll(). A pair differs
   // while an event of that type waits in the queue.
   std::atomic<uint8_t> pendingAddress_{ kNoAddress };
   std::atomic<uint8_t> resetsQueued_{ 0u };
   std::atomic<uint8_t> setupsQueued_{ 0u };
   uint8_t resetsDelivered_ = 0u;
   uint8_t setupsDelivered_ = 0u;
   std::atomic<uint32_t> droppedEvents_{ 0u };
   // Out and InComplete events per endpoint direction (OUT 0..15, IN 16..31).
   std::array<std::atomic<uint8_t>, kSlots> packetsQueued_{};
   std::array<uint8_t, kSlots> packetsDelivered_{};

   // Main loop only. staleEvents_: packet events still queued when the endpoint was closed; poll() drops them.
   // deferredArms_: OUT endpoints (bit per number) that armOutTransfer() arms after poll() has delivered their
   // queued packet.
   std::array<uint8_t, kSlots> staleEvents_{};
   uint16_t deferredArms_ = 0u;

   static void onInterruptEvent(void *context, const Event &event);
   void handleInterruptEvent(const Event &event);
   void deliver(const Event &event);
   [[nodiscard]] bool isSupersededControlCall(uint8_t endpointAddress) const;
   [[nodiscard]] bool isOpen(uint8_t endpointAddress) const;
   [[nodiscard]] bool isOutQueued(uint8_t endpointAddress) const;
   void armDeferredOut(uint8_t endpointAddress);
};

#endif
