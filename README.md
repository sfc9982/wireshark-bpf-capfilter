# wireshark-bpf-capfilter
Generate a BPF-style Wireshark Capture Filter

```
String-Matching Capture Filter Generator
Usage:
	--str    / -s <string you want to match>
	--offset / -o <offset from the start of the TCP data>
  ```
  
`./wireshark_capfilter --str "POST" --offset 0`

```
Output:
tcp[((tcp[12:1] & 0xf0) >> 2):4] = 0x504f5354
```
