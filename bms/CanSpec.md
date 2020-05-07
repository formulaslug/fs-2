# BMS CAN Specifications

This defines the CAN specifications for the BMS.

| ID     | Byte_0                | Byte_1                 | Byte_2                | Byte_3                 | Byte_4                | Byte_5                 | Byte_6                | Byte_7                 | Comments  |
|--------|-----------------------|------------------------|-----------------------|------------------------|-----------------------|------------------------|-----------------------|------------------------|-----------|
| 0x0420 | 'S'                   | 'P'                    | 'I'                   | 'C'                    | 'Y'                   | 'B'                    | 'O'                   | 'I'                    | I HATE U  |
| 0x0421 | Contractor State      | Fault State            | Warning State         | SoC                    | DoD [0:7]             | DoD [8:15]             | Total Current [0:7]   | Total Current [8:15]   |           |
| 0x0422 | Total Voltage [0:7]   | Total Voltage [8:15]   | Max Voltage [0:7]     | Max Voltage [8:15]     | Min Voltage [0:7]     | Min Voltage [8:15]     | Max Temp              | Min Temp               |           |
| 0x0423 | Voltage Cell 1 [0:7]  | Voltage Cell 1 [8:15]  | Voltage Cell 2 [0:7]  | Voltage Cell 2 [8:15]  | Voltage Cell 3 [0:7]  | Voltage Cell 3 [8:15]  | Voltage Cell 4 [0:7]  | Voltage Cell 4 [8:15]  |           |
| 0x0424 | Voltage Cell 5 [0:7]  | Voltage Cell 5 [8:15]  | Voltage Cell 6 [0:7]  | Voltage Cell 6 [8:15]  | Voltage Cell 7 [0:7]  | Voltage Cell 7 [8:15]  | Voltage Cell 8 [0:7]  | Voltage Cell 8 [8:15]  |           |
| 0x0425 | Voltage Cell 9 [0:7]  | Voltage Cell 9 [8:15]  | Voltage Cell 10 [0:7] | Voltage Cell 10 [8:15] | Voltage Cell 11 [0:7] | Voltage Cell 11 [8:15] | Voltage Cell 12 [0:7] | Voltage Cell 12 [8:15] |           |
| 0x0426 | Voltage Cell 13 [0:7] | Voltage Cell 13 [8:15] | Voltage Cell 14 [0:7] | Voltage Cell 14 [8:15] | Voltage Cell 15 [0:7] | Voltage Cell 15 [8:15] | Voltage Cell 16 [0:7] | Voltage Cell 16 [8:15] |           |
| 0x0427 | Voltage Cell 17 [0:7] | Voltage Cell 17 [8:15] | Voltage Cell 18 [0:7] | Voltage Cell 18 [8:15] | Voltage Cell 19 [0:7] | Voltage Cell 19 [8:15] | Voltage Cell 20 [0:7] | Voltage Cell 20 [8:15] |           |
| 0x0428 | Voltage Cell 21 [0:7] | Voltage Cell 21 [8:15] | Voltage Cell 22 [0:7] | Voltage Cell 22 [8:15] | Voltage Cell 23 [0:7] | Voltage Cell 23 [8:15] | Voltage Cell 24 [0:7] | Voltage Cell 24 [8:15] |           |
| 0x0429 | Voltage Cell 25 [0:7] | Voltage Cell 25 [8:15] | Voltage Cell 26 [0:7] | Voltage Cell 26 [8:15] | Voltage Cell 27 [0:7] | Voltage Cell 27 [8:15] | Voltage Cell 28 [0:7] | Voltage Cell 28 [8:15] |           |
| 0x042a | Temp Cell 1           | Temp Cell 2            | Temp Cell 3           | Temp Cell 4            | Temp Cell 5           | Temp Cell 6            | Temp Cell 7           | -                      |           |
| 0x042b | Temp Cell 8           | Temp Cell 9            | Temp Cell 10          | Temp Cell 11           | Temp Cell 12          | Temp Cell 13           | Temp Cell 14          | -                      |           |
| 0x042c | Temp Cell 15          | Temp Cell 16           | Temp Cell 17          | Temp Cell 18           | Temp Cell 19          | Temp Cell 20           | Temp Cell 21          | -                      |           |
| 0x042d | Temp Cell 22          | Temp Cell 23           | Temp Cell 24          | Temp Cell 25           | Temp Cell 26          | Temp Cell 27           | Temp Cell 28          | -                      |           |
