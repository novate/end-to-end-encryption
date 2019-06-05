## Packet Design

|     PacketType     | Descriptor | Done | Check |
| :----------------: | :--------: | :--: | ----- |
|   VersionRequire   |    0x00    |  √   |       |
|    AuthRequest     |    0x01    |      |       |
|    AuthResponse    |    0x01    |      |       |
|   SysInfoRequest   |    0x02    |      |       |
|  SysInfoResponse   |    0x02    |      |       |
|  ConfInfoRequest   |    0x03    |      |       |
|  ConfInfoResponse  |    0x03    |      |       |
|  ProcInfoRequest   |    0x04    |      |       |
|  ProcInfoResponse  |    0x04    |      |       |
|  EtherInfoRequest  |    0x05    |      |       |
| EtherInfoResponse  |    0x05    |      |       |
|   USBInfoRequest   |    0x07    |      |       |
|  USBInfoResponse   |    0x07    |      |       |
|   USBfileRequest   |    0x0C    |      |       |
|  USBfileResponse   |    0x0C    |      |       |
|  PrintDevRequest   |    0x08    |      |       |
|  PrintDevResponse  |    0x08    |      |       |
| PrintQueueRequest  |    0x0D    |      |       |
| PrintQueueResponse |    0x0D    |      |       |
|   TerInfoRequest   |    0x09    |      |       |
|  TerInfoResponse   |    0x09    |      |       |
|   DumbTerRequest   |    0x0A    |      |       |
|  DumbTerResponse   |    0x0A    |      |       |
|   IPTermRequest    |    0x0B    |      |       |
|   IPTermResponse   |    0x0B    |      |       |
|        End         |    0xFF    |      |       |
|                    |            |      |       |





____________

### VersionRequire 0x00

![1559727225112](C:\Users\UCRRR\AppData\Roaming\Typora\typora-user-images\1559727225112.png)



| Data                           | Format |
| ------------------------------ | ------ |
| required lowest server version |        |
|                                |        |
|                                |        |

