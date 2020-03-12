## Packet Design

Client:

|     PacketType     | Descriptor |
| :----------------: | :--------: |
|   VersionRequire   |    0x00    |
|    AuthRequest     |    0x01    |
|    AuthResponse    |    0x01    |
|   SysInfoRequest   |    0x02    |
|  SysInfoResponse   |    0x02    |
|  ConfInfoRequest   |    0x03    |
|  ConfInfoResponse  |    0x03    |
|  ProcInfoRequest   |    0x04    |
|  ProcInfoResponse  |    0x04    |
|  EtherInfoRequest  |    0x05    |
| EtherInfoResponse  |    0x05    |
|   USBInfoRequest   |    0x07    |
|  USBInfoResponse   |    0x07    |
|   USBfileRequest   |    0x0C    |
|  USBfileResponse   |    0x0C    |
|  PrintDevRequest   |    0x08    |
|  PrintDevResponse  |    0x08    |
| PrintQueueRequest  |    0x0D    |
| PrintQueueResponse |    0x0D    |
|   TerInfoRequest   |    0x09    |
|  TerInfoResponse   |    0x09    |
|   DumbTerRequest   |    0x0A    |
|  DumbTerResponse   |    0x0A    |
|   IPTermRequest    |    0x0B    |
|   IPTermResponse   |    0x0B    |
|        End         |    0xFF    |

### Encrypt

(116 bytes in total)

1. Host to Network: 8 - 80 bytes
2. Encrypt and replace: 8 - 112 bytes
3. Host to Network: 112 - 116 bytes

### Decrypt

1. Network to Host: 112 - 116 bytes
2. Decrypt and replace: 8 - 112 bytes
3. Network to Host: 8 - 80 bytes
