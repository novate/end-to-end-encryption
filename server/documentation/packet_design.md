# Simple Chat Room Package Design  

## Frame type (all)

|  frame type  | descriptor |
| :-----------------: | :--------: |
| info | 0x00 |
| info_response | 0x01 |
| passwd | 0x02 |
| passwd_response | 0x03 |
| refuse | 0x04 |
| UserName | 0x06 |
| OnlineUser | 0x07 |
| SyncEnd | 0x08 |
|   SendInvit   |    0x09    |
|   RecvInvit   |    0x0A    |
| InvitResponse |    0x0B    |
|     Board     |    0x0C    |
|  SingleCoord  |    0x0D    |
|  DoubleCoord  |    0x0E    |
|   GameOver    |    0x0F    |
| OfflineUser | 0x10 |

------------------------

## Log In

### Frame types(Log In)

|   frame type    | descriptor |
| :-------------: | :--------: |
|      info       |    0x00    |
|  info_response  |    0x01    |
|     passwd      |    0x02    |
| passwd_response |    0x03    |
|     refuse      |    0x04    |

------------------------

#### info `0x00`

|  0   |         1 , 2         |     3 ... 31      |
| :--: | :-------------------: | :---------------: |
| 0x00 | info_length (2 bytes) | data（user_name） |

#### info_response `0x01`

|  0   |  1, 2  |      3       |
| :--: | :----: | :----------: |
| 0x01 | 0x0001 | ResponseType |

 > **ResponseType:**
 >
 > > >|         0x0         | 0x1  |     0x4     |
 > > >| :-----------------: | :--: | :---------: |
 > > >| user does not exist |  OK  | ErrorOccurs |

#### passwd `0x02`

|  0   |          1 , 2          |        3 ... 31         |
| :--: | :---------------------: | :---------------------: |
| 0x02 | passwd_length (2 bytes) | password (28 bytes max) |

#### passwd_response `0x03`

|  0   |  1, 2  |      3       |
| :--: | :----: | :----------: |
| 0x03 | 0x0001 | ResponseType |

> **ResponseType:**

> | 0x1  |  0x3  |     0x4     |
> | :--: | :---: | :---------: |
> |  OK  | wrong | ErrorOccurs |
> 密码错误client不直接踢掉

#### refuse  `0x04`

|  0   |  1, 2  |      3       |
| :--: | :----: | :----------: |
| 0x04 | 0x0001 | responseType |

>**ResponseType:**
>
>|   **0x4**   |     **0x5**     |
>| :---------: | :-------------: |
>| ErrorOccurs | AlreadyLoggedIn |





-----------

## Synchronization

### Frame types

| frame type  | descriptor |
| :---------: | :--------: |
|  UserName   |    0x06    |
| OnlineUser  |    0x07    |
|   SyncEnd   |    0x08    |
| OfflineUser |    0x10    |


#### **UserName**  `0x06`

|  0   |            1, 2            |         3 ... 31         |
| :--: | :------------------------: | :----------------------: |
| 0x06 | user_name length (2 bytes) | user_name (host to user) |


#### **OnlineUser** `0x07`

|  0   |            1,2            |        3... 31        |
| :--: | :-----------------------: | :-------------------: |
| 0x07 | user_name length(2 bytes) | user_name (broadcast) |

#### **SyncEnd**   `0x08`

1 byte

|  0   | 1， 2  |  3   |
| :--: | :----: | :--: |
| 0x08 | 0x0000 |  0   |

#### **OfflineUser** `0x10`

|  0   |            1,2            |        3... 31        |
| :--: | :-----------------------: | :-------------------: |
| 0x10 | user_name length(2 bytes) | user_name (broadcast) |

------------------------

## Game

**Frame types**

|  frame type   | descriptor |
| :-----------: | :--------: |
|   SendInvit   |    0x09    |
|   RecvInvit   |    0x0A    |
| InvitResponse |    0x0B    |
|     Board     |    0x0C    |
|  SingleCoord  |    0x0D    |
|  DoubleCoord  |    0x0E    |
|   GameOver    |    0x0F    |

**SendInvit** **0x09**

|  0   |            1,2             |         3... 31          |
| :--: | :------------------------: | :----------------------: |
| 0x09 | user_name length (2 bytes) | user_name (host to user) |


**RecvInvit** **0x0A**

|  0   |            1,2             |         3... 31          |
| :--: | :------------------------: | :----------------------: |
| 0x0A | user_name length (2 bytes) | user_name (host to user) |

**InvitResponse** **0x0B**

|  0   | 1,2  |      3       |
| :--: | :--: | :----------: |
| 0x0A |  1   | ResponseType |

  >**ResponseType:**
  >
  >|      0       |  1   |      2      |  6   |
  >| :----------: | :--: | :---------: | :--: |
  >| UserNotExist |  OK  | RefuseInvit | Busy |

 

**Board 0x0C**

|  0   | 1,2  |                           3... 14                            |
| :--: | :--: | :----------------------------------------------------------: |
| 0x0C |  12  | $ h_1(x,y), t_1(x,y), h_2(x,y), t_2(x,y), h_3(x,y), t_3(x,y) $ |



**SingleCoord 0x0D**

|  0   | 1,2  | 3,4  |
| :--: | :--: | :--: |
| 0x0D |  2   | x,y  |



**DoubleCoord 0x0E**

|  0   | 1,2  |          3,4          |
| :--: | :--: | :-------------------: |
| 0x0E |  4   | $ h_x, h_y, t_x,t_y $ |



**GameOver 0x0F**

|  0   | 1,2  |  3   |
| :--: | :--: | :--: |
| 0x0F |  0   | null |

>If you win, your client will send frame GameOver to the server;
>
>If you lose, your client will recv frame GameOver from the server.