
# CommunicationManager
The CommunicationManager provides a simple interface to communicate over CAN. It follows the Publisher-Subscriber pattern.
A publisher is offering messages to all other network nodes. Subscribers can subscribe to messages which they are interested in.
The library is built on top of [FlexCAN](https://github.com/pawelsky/FlexCAN_Library).

## Required Hardware
You need a teensy 3.2 board and a CAN transreceiver (e.g. SN65HVD230) to use this library.

## How to use this library
Just take a look into the [examples](examples/) to see how to use the CommunicationManager class. Please keep in mind that the maximum message size is limited to 8 bytes.

**Provided functions:**
<table class="tg">
  <tr>
    <th class="tg-1wig">Function</th>
    <th class="tg-1wig">Parameters</th>
    <th class="tg-1wig">Return value</th>
    <th class="tg-0lax">Description</th>
  </tr>
  <tr>
    <td class="tg-0lax">static CommunicationManager* GetInstance();</td>
    <td class="tg-0lax">-</td>
    <td class="tg-0lax">Pointer to the class instance</td>
    <td class="tg-0lax"></td>
  </tr>
  <tr>
    <td class="tg-0lax">void Initialize(uint32_t baud = 500000);</td>
    <td class="tg-0lax"><b>baud:</b> Speed in bits per second</td>
    <td class="tg-0lax">-</td>
    <td class="tg-0lax">Initializes the CommunicationManager. Should be called in the setup() Method of your sketch</td>
  </tr>
  <tr>
    <td class="tg-0lax">bool Fire(unsigned int canId);</td>
    <td class="tg-0lax"><b style="font-weight:bold">canId:</b><b style="font-weight:normal"> </b>CAN Identifier</td>
    <td class="tg-0lax">False if an error occured, otherwise true</td>
    <td class="tg-0lax">Writes a CAN message immediately into the message queue</td>
  </tr>
  <tr>
    <td class="tg-0lax">bool Fire(void* val, unsigned int bytes, unsigned int canId);</td>
    <td class="tg-0lax"><b style="font-weight:bold">val:</b>  Pointer to value<br><br><b style="font-weight:bold">bytes:</b> Number of bytes<br><br><b style="font-weight:bold">canId:</b> CAN Identifier</td>
    <td class="tg-0lax">False if an error occured, otherwise true</td>
    <td class="tg-0lax">Writes a CAN message immediately into the message queue</td>
  </tr>
  <tr>
    <td class="tg-0lax">bool Publish(void* val, unsigned int bytes, unsigned int canId, unsigned char* txFlag, COM_CYCLE cycle);</td>
    <td class="tg-0lax"><b style="font-weight:bold">val:</b> Pointer to value<br><br>
	<b style="font-weight:bold">bytes:</b> Number of bytes<br><br><b style="font-weight:bold">canId:</b> CAN Identifier<br><br>
	<b style="font-weight:bold">txFlag:</b> Pointer to transmitted flag<br><br><b style="font-weight:bold">cycle:</b> Send cycletime<br><td class="tg-0lax">False if an error occured, otherwise true</td>
    <td class="tg-0lax">Publishes value with the given CAN Identifier with specified cycle time. The flag gets set to '1' everytime the value was sent</td>
  </tr>
  <tr>
    <td class="tg-0lax">bool Subscribe(void* val, unsigned int bytes, unsigned int canId, unsigned char* rxFlag);</td>
    <td class="tg-0lax"><b style="font-weight:bold">val:</b> Pointer to value<br><br>
	<b style="font-weight:bold">bytes:</b> Number of bytes<br><br><b style="font-weight:bold">canId:</b> CAN Identifier<br><br>
	<b style="font-weight:bold">rxFlag:</b> Pointer to received flag<br></td>
    <td class="tg-0lax">False if an error occured, otherwise true</td>
    <td class="tg-0lax">Subscribes to a CAN message and writes the received payload into value. The flag gets set to '1' everytime a message was received</td>
  </tr>
</table>
