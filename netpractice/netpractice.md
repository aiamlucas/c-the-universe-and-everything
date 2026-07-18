# NetPractice
 
Everything you need to understand and solve all 10 levels of NetPractice: binary math, IP addressing, subnet masks, CIDR, routing, and the practical tricks used during the defense.
 
---
 
## Table of Contents
 
0. [The Master Table (everything in one place)](#0-the-master-table-everything-in-one-place)
1. [Binary & Bitwise Basics](#1-binary--bitwise-basics)
2. [What is an IP Address (IPv4)](#2-what-is-an-ip-address-ipv4)
3. [The Subnet Mask](#3-the-subnet-mask)
4. [CIDR Notation](#4-cidr-notation)
5. [Network, Host, and Broadcast Addresses](#5-network-host-and-broadcast-addresses)
6. [Subnetting: The Core Skill](#6-subnetting-the-core-skill)
7. [Private & Reserved IP Ranges](#7-private--reserved-ip-ranges)
8. [Network Devices: Hub, Switch, Router](#8-network-devices-hub-switch-router)
9. [The Default Gateway](#9-the-default-gateway)
10. [Routing Tables](#10-routing-tables)
11. [The TCP/IP Model (and OSI)](#11-the-tcpip-model-and-osi)
12. [How a Packet Travels: Putting It All Together](#12-how-a-packet-travels-putting-it-all-together)
13. [NetPractice Level-by-Level Strategy](#13-netpractice-level-by-level-strategy)
14. [Cheat Sheets & Mental Math Tricks](#14-cheat-sheets--mental-math-tricks)
15. [Common Errors in the Logs & What They Mean](#15-common-errors-in-the-logs--what-they-mean)
16. [Appendix: CIDR /24 → /32, the Bit-Value Pattern](#16-appendix-cidr-24--32-the-bit-value-pattern)
17. [Appendix: Switch & Router in ASCII](#17-appendix-switch--router-in-ascii)
---
 
## 0. The Master Table (everything in one place)
 
Each column is one bit of the last octet. Reading top to bottom: the bit's value, the CIDR mask that *ends* on that bit, the resulting mask octet (cumulative sum of all values to the left, including this one), the block size (= the bit's own value), and the usable hosts.
 
| Bit position          | 7    | 6    | 5    | 4    | 3    | 2    | 1    | 0    |
|-----------------------|------|------|------|------|------|------|------|------|
| **Bit value**         | 128  | 64   | 32   | 16   | 8    | 4    | 2    | 1    |
| **CIDR**              | /25  | /26  | /27  | /28  | /29  | /30  | /31  | /32  |
| **Mask octet**        | .128 | .192 | .224 | .240 | .248 | .252 | .254 | .255 |
| **Block size**        | 128  | 64   | 32   | 16   | 8    | 4    | 2    | 1    |
| **Usable hosts**      | 126  | 62   | 30   | 14   | 6    | 2    | 0    | —    |
 
(And /24 = mask .0, block 256, 254 hosts — the row "before" this table starts.)
 
How to read it, with /27 as an example:
- **Mask octet** = sum of bit values from the left up to /27's column: 128+64+32 = **224** → mask 255.255.255.224.
- **Block size** = the bit value in /27's own column = **32** → subnets at .0, .32, .64, .96, ...
- **Usable hosts** = block − 2 = **30**.
Notice the symmetry: *block size and bit value are the same number*, and mask octet + block size = 256 in every column. That means the table also encodes the `256 − mask` trick — you never need to compute it, just read the column.
 
The same sequence shifts one octet left for /17–/23 (third octet) and again for /9–/15 (second octet):
 
| CIDR      | /17  | /18  | /19  | /20  | /21  | /22  | /23  |
|-----------|------|------|------|------|------|------|------|
| 3rd octet | .128 | .192 | .224 | .240 | .248 | .252 | .254 |
 
---

## 1. Binary & Bitwise Basics

Computers store everything in **bits** (0 or 1). An IPv4 address is just **32 bits**, grouped into 4 bytes (octets) of 8 bits each.

### Decimal ↔ Binary for one octet

Each bit in an octet has a positional value:

| Bit position | 7   | 6  | 5  | 4  | 3 | 2 | 1 | 0 |
|--------------|-----|----|----|----|---|---|---|---|
| Value        | 128 | 64 | 32 | 16 | 8 | 4 | 2 | 1 |

To convert binary → decimal, add up the values where a `1` appears:

```
11000000 = 128 + 64             = 192
10101000 = 128 + 32 + 8         = 168
00001010 = 8 + 2                = 10
11111111 = 128+64+32+16+8+4+2+1 = 255
```

To convert decimal → binary, subtract the biggest power of 2 that fits, repeatedly:

```
200 → 200-128=72 → 72-64=8 → 8-8=0
    → bits: 128 ✓, 64 ✓, 32 ✗, 16 ✗, 8 ✓, 4 ✗, 2 ✗, 1 ✗
    → 11001000
```

### Bitwise operations you need

| A | B | A & B (AND) | A \| B (OR) | A ^ B (XOR) |
|---|---|-------------|-------------|-------------|
| 0 | 0 |      0      |      0      |      0      |
| 0 | 1 |      0      |      1      |      1      |
| 1 | 0 |      0      |      1      |      1      |
| 1 | 1 |      1      |      1      |      0      |

**AND (`&`)** — the result bit is 1 only if *both* input bits are 1:

```
  11000000  (192)
& 10100000  (160)
= 10000000  (128)
```

**OR (`|`)** — result bit is 1 if *either* bit is 1.

**NOT (`~`)** — flips every bit: `~11110000 = 00001111`.

Why you care: **the network address is computed by ANDing the IP with the subnet mask**, and the broadcast address uses OR with the inverted mask. That single fact drives the entire project.

---

## 2. What is an IP Address (IPv4)

An IPv4 address is a **32-bit number** written as four decimal octets separated by dots:

```
192.168.1.10
   ↓ in binary
11000000.10101000.00000001.00001010
```

Every IP address is split into two logical parts:

- **Network part** — identifies *which network* the device is on (like a street name).
- **Host part** — identifies *which device* on that network (like a house number).

Where the split happens is defined by the **subnet mask**.

Two devices can talk **directly** (without a router) only if they are on the **same network** — meaning their network parts are identical.

---

## 3. The Subnet Mask

The subnet mask is also 32 bits: a run of consecutive `1`s followed by consecutive `0`s.

- `1` bits mark the **network** portion.
- `0` bits mark the **host** portion.

```
Mask 255.255.255.0
  = 11111111.11111111.11111111.00000000
    └── network (24 bits) ───┘ └ host ┘(8 bits) 
```

A mask like `255.255.0.255` is **invalid** — the 1s must be contiguous from the left.

### Valid mask octet values

Because the 1s are contiguous, each octet of a mask can only be one of these 9 values:


| Binary   | Dec | 1-bits |  if 4th octet | if 3rd octet | if 2nd octet |
|----------|-----|--------|---------------|--------------|--------------|
| 00000000 |   0 |   0    |      /24      |     /16      |     /8       |
| 10000000 | 128 |   1    |      /25      |     /17      |     /9       |
| 11000000 | 192 |   2    |      /26      |     /18      |     /10      |
| 11100000 | 224 |   3    |      /27      |     /19      |     /11      |
| 11110000 | 240 |   4    |      /28      |     /20      |     /12      |
| 11111000 | 248 |   5    |      /29      |     /21      |     /13      |
| 11111100 | 252 |   6    |      /30      |     /22      |     /14      |
| 11111110 | 254 |   7    |      /31      |     /23      |     /15      |
| 11111111 | 255 |   8    |      /32      |     /24      |     /16      |

**Memorize this table.** It's the single most useful table in NetPractice.

### Finding the network address

Network address = IP **AND** mask.

```
IP:   192.168.  1. 10  → ...00001010
Mask: 255.255.255.  0  → ...00000000
AND:  192.168.  1.  0  ← network address
```

Two hosts are on the same network **iff** `IP1 & mask == IP2 & mask`.

---

## 4. CIDR Notation

Instead of writing the whole mask, we write `/N` where N = number of `1` bits.

```
/24 = 255.255.255.0
/25 = 255.255.255.128
/30 = 255.255.255.252
/16 = 255.255.0.0
```

Conversion rule: divide N by 8. The quotient tells you how many octets are 255; the remainder tells you the value of the next octet (look it up in the table above).

```
/27 → 27 = 8+8+8+3 → 255.255.255.(11100000) → 255.255.255.224
/18 → 18 = 8+8+2   → 255.255.(11000000).0   → 255.255.192.0
```

### Hosts per mask

Host bits = 32 − N. Number of *usable* host addresses = 2^(host bits) − 2 (minus the network and broadcast addresses).

| CIDR | Mask | Host bits | Usable hosts | Block size (last octet) |
|---|---|---|---|---|
| /24 | 255.255.255.0 | 8 | 254 | 256 |
| /25 | 255.255.255.128 | 7 | 126 | 128 |
| /26 | 255.255.255.192 | 6 | 62 | 64 |
| /27 | 255.255.255.224 | 5 | 30 | 32 |
| /28 | 255.255.255.240 | 4 | 14 | 16 |
| /29 | 255.255.255.248 | 3 | 6 | 8 |
| /30 | 255.255.255.252 | 2 | 2 | 4 |

**/30 is everywhere in NetPractice** — it's the classic mask for a point-to-point link between two routers (exactly 2 usable hosts).

---

## 5. Network, Host, and Broadcast Addresses

For any subnet, two addresses are **reserved and unusable for hosts**:

- **Network address** — all host bits set to 0. Identifies the subnet itself.
- **Broadcast address** — all host bits set to 1. Used to reach every host in the subnet.

Example with `192.168.1.0/26` (mask 255.255.255.192, block size 64):

```
Network:    192.168.1.0      (host bits all 0)
First host: 192.168.1.1
Last host:  192.168.1.62
Broadcast:  192.168.1.63     (host bits all 1)
Next subnet starts at: 192.168.1.64
```

**In NetPractice, assigning a host the network or broadcast address is a classic trap.** If the range is `.0`–`.63`, valid host IPs are `.1` through `.62` only.

> This is also where a note like "190 is the max" comes from: in the subnet
> `10.0.0.128/26`, hosts run from **.129 to .190** and **.191** is the broadcast.
> The number 190 is not a general rule — it's just the last usable host of *that*
> particular block. Always derive it from the block, never memorize a magic number.

### The "magic number" / block size trick

Block size = 256 − (mask value in the "interesting" octet).

```
Mask 255.255.255.224 → interesting octet = 224 → block = 256-224 = 32
Subnets: .0–.31, .32–.63, .64–.95, .96–.127, ...

Which subnet contains 192.168.1.75 with /27?
75 falls in the .64–.95 block → network = .64, broadcast = .95,
hosts = .65 to .94
```

This trick replaces all binary math during the timed defense.

---

## 6. Subnetting: The Core Skill

Subnetting = splitting one network into several smaller ones by **borrowing host bits** and turning them into network bits (increasing /N).

Example: split `10.0.0.0/24` into 4 subnets:

- Need 4 subnets → borrow 2 bits (2² = 4) → new mask = /26.

```
Subnet 1: 10.0.0.0/26    hosts .1–.62,    broadcast .63
Subnet 2: 10.0.0.64/26   hosts .65–.126,  broadcast .127
Subnet 3: 10.0.0.128/26  hosts .129–.190, broadcast .191
Subnet 4: 10.0.0.192/26  hosts .193–.254, broadcast .255
```

Key rule for NetPractice: **subnets must never overlap**. If two different interfaces of a router are on overlapping ranges, the level fails. When you design masks yourself, make sure each router interface sits in a distinct, non-overlapping subnet.

Checklist for "are A and B on the same network?":

1. Take A's mask, compute A & mask.
2. Compute B & (same mask... but also check with B's own mask — both directions must agree in practice).
3. If the network addresses match, they can talk directly.

---

## 7. Private & Reserved IP Ranges

Some ranges are reserved and matter in NetPractice, especially in levels where a device connects to "the Internet":

| Range                         | CIDR           | Purpose              |
|-------------------------------|----------------|----------------------|
| 10.0.0.0 – 10.255.255.255     | 10.0.0.0/8     | Private              |
| 172.16.0.0 – 172.31.255.255   | 172.16.0.0/12  | Private              |
| 192.168.0.0 – 192.168.255.255 | 192.168.0.0/16 | Private              |
| 127.0.0.0 – 127.255.255.255   | 127.0.0.0/8    | Loopback (localhost) |
| 169.254.0.0/16                | —              | Link-local (APIPA)   |
| 224.0.0.0 – 239.255.255.255   | —              | Multicast            |

**NetPractice rule of thumb:** private addresses are **not routable on the Internet**. In levels featuring an Internet cloud, any interface that must be reached *from* the Internet cannot use a private range (the logs will complain "private address"). Also never use loopback (127.x.x.x) or multicast addresses for hosts.

> **Loopback, properly defined:** the loopback address (127.0.0.1, "localhost") is a
> *virtual* interface a machine uses to talk to **itself** — traffic sent there never
> leaves the device. It is not just "an address on a non-physical interface"; it is
> specifically self-addressed. NetPractice rule: **never assign 127.x.x.x to any
> interface in the exercises.**

---

## 8. Network Devices: Hub, Switch, Router

### Hub
- The "dumb" ancestor of the switch (layer 1). It blindly repeats every incoming signal to **all** ports.
- No MAC learning, no IP, no intelligence. You won't configure hubs in NetPractice, but knowing the contrast helps: a switch is a hub that learned to read addresses.

### Switch
- Operates at **layer 2** (data link, MAC addresses). It learns which MAC address lives on which port and forwards frames only where they need to go.
- Uses **ARP** traffic to do its job: when host A wants to reach IP 192.168.1.20 on its own subnet, it broadcasts an ARP request — *"who has 192.168.1.20? Tell me your MAC address"* — and the owner replies with its MAC. From then on, frames go directly to that MAC.
- **Has no IP address** in NetPractice and no configuration.
- Simply connects multiple devices into the **same network**: everything plugged into a switch must share one subnet (same network address, same mask).

### Router
- Operates at **layer 3** (network, IP addresses).
- **Connects different networks.** Each router **interface** has its own IP and belongs to a **different subnet**.
- Golden rule: **the interfaces of one router must all be in different, non-overlapping subnets.**

### Host (computer)
- Has one or more interfaces, each with an IP + mask, and usually a **routing table** with a default route pointing to a gateway.

Mental model:

```
[Host A] ──┐
[Host B] ──┼──[ Switch ]──── same subnet ────[ Router iface 1 ]
[Host C] ──┘                                  [ Router iface 2 ]──── other subnet ──── ...
```

---

## 9. The Default Gateway

When a host wants to send a packet to an IP **outside its own subnet**, it can't deliver it directly. It hands the packet to its **default gateway** — the router interface on *its own* subnet.

Rules that NetPractice enforces:

1. The gateway address in a host's route **must be inside the host's own subnet** (same network as the host's interface).
2. The gateway must be the IP of the **router interface directly connected** to that subnet — not some far-away router.
3. Written in the routing table as destination `default` (or `0.0.0.0/0`) → next hop = gateway IP.

```
Host: 192.168.1.10/24
Router interface on that LAN: 192.168.1.1
Host route: default → 192.168.1.1   ✓

default → 192.168.5.1  ✗ (not in the host's subnet → "gateway unreachable")
```

---

## 10. Routing Tables

A routing table is a list of rules: **destination → next hop**.

```
destination            next hop
192.168.42.0/24   →    10.1.1.2
default (0.0.0.0/0) →  10.1.1.1
```

How a device uses it:

1. Look at the packet's destination IP.
2. Find the most specific matching destination entry (longest prefix wins).
3. Send the packet to the corresponding **next hop**, which must be directly reachable (in one of the device's own subnets).
4. `default` matches everything not matched by a more specific route.

NetPractice specifics:

- In route fields, `default` and `0.0.0.0/0` are equivalent.
- The **next hop must be on the same network as one of the device's interfaces** — otherwise the route is useless.
- Routers need routes **in both directions**: R1 must know how to reach the destination network, and the routers on the way back must know how to reach the *source* network. **A very common bug in the later levels is a working forward path with no return route.**
- The Internet cloud in NetPractice also has a routing table; you often must add a route there telling the Internet how to reach your (public) network.

---

## 11. The TCP/IP Model (and OSI)

You mainly need layer 2 vs layer 3 intuition, but for the README/defense here's the theory.

### OSI model (7 layers)

| # | Layer | Role | Example |
|---|---|---|---|
| 7 | Application | User-facing protocols | HTTP, DNS, SSH |
| 6 | Presentation | Encoding, encryption | TLS, JPEG |
| 5 | Session | Session management | — |
| 4 | Transport | End-to-end delivery, ports | TCP, UDP |
| 3 | Network | Logical addressing, routing | IP, ICMP — **routers live here** |
| 2 | Data link | Local delivery via MAC | Ethernet, ARP — **switches live here** |
| 1 | Physical | Bits on the wire | Cables, radio — **hubs live here** |

### TCP/IP model (4 layers)

| TCP/IP layer | Maps to OSI | Protocols |
|---|---|---|
| Application | 5–7 | HTTP, DNS, FTP |
| Transport | 4 | TCP, UDP |
| Internet | 3 | IP, ICMP |
| Link | 1–2 | Ethernet, ARP |

**TCP** = reliable, ordered, connection-based (handshake). **UDP** = fast, connectionless, no delivery guarantee. **ICMP** is what `ping` uses — and conceptually what NetPractice simulates when it checks "A can reach B".

---

## 12. How a Packet Travels: Putting It All Together

Host A (192.168.1.10/24, gw 192.168.1.1) sends to 172.16.5.5:

1. A computes: is 172.16.5.5 in my network? `172.16.5.5 & 255.255.255.0` ≠ `192.168.1.0` → **no**.
2. A sends the packet to its default gateway 192.168.1.1 (router R1's interface).
3. R1 checks its routing table for a route matching 172.16.5.5 → forwards to next hop, e.g. R2.
4. R2 has an interface directly on 172.16.5.0/24 → delivers directly to the host.
5. The reply travels the same way **in reverse** — every router on the path must also have a route back to 192.168.1.0/24.

If any hop lacks a matching route, or a gateway isn't in the right subnet, or two subnets overlap — the packet dies, and NetPractice logs tell you where.

---

## 13. NetPractice Level-by-Level Strategy

The exact exercises vary per configuration, but the levels follow this progression:

- **Levels 1–2 — same subnet basics.** Two hosts through a switch or direct link. Make both IPs share the network part under the given mask, keep host parts different, avoid network/broadcast addresses. If masks are editable, they must match on both ends.
- **Level 3 — mask manipulation.** One side's mask is fixed (e.g. /25); the other fields must conform. Compute the range implied by the fixed mask and fit everything inside it.
- **Levels 4–5 — router interfaces & gateways.** Each router interface = its own subnet. Set host default routes to the local router interface. Watch for overlapping subnets on the router.
- **Level 6 — the Internet appears.** The Internet needs a route to your network; your public-facing addresses can't be private ranges.
- **Level 7 — subnet splitting.** You must carve one address space into multiple non-overlapping subnets yourself (typically /30s and small blocks). This is where the block-size trick shines.
- **Levels 8–9 — multiple routers + Internet routes.** Chain of routers; every router (and the Internet) needs correct forward *and* return routes. Trace the packet in both directions on paper.
- **Level 10 — everything combined, many free fields.** Pick a clean plan first (write down each subnet range), then fill fields. Don't improvise field by field.

General method for any level:

1. **Read the goal** at the top (who must reach whom).
2. **Identify fixed fields** (shaded) — they constrain everything. Work outward from them.
3. For each fixed mask, compute the **subnet range** (block size trick).
4. Assign IPs: same subnet across each switch/link, different subnets across each router.
5. Fill routing tables: hosts get `default → local router iface`; routers get routes to every non-directly-connected network, **both directions**.
6. Click **Check again**, read the **logs**, fix, repeat.
7. **Export with "Get my config"** before moving on (needed for submission!).

---

## 14. Cheat Sheets & Mental Math Tricks

### Powers of 2
```
2^1=2  2^2=4  2^3=8  2^4=16  2^5=32  2^6=64  2^7=128  2^8=256
```

### Mask ↔ CIDR ↔ block size (last octet)
```
/24 → 255.255.255.0    block 256   254 hosts
/25 → .128             block 128   126 hosts
/26 → .192             block  64    62 hosts
/27 → .224             block  32    30 hosts
/28 → .240             block  16    14 hosts
/29 → .248             block   8     6 hosts
/30 → .252             block   4     2 hosts  ← router-to-router links
```

### Instant subnet range
1. Block size = 256 − mask octet.
2. Network = largest multiple of block size ≤ the IP's octet.
3. Broadcast = network + block − 1.
4. Usable hosts = everything strictly between them.

```
IP 10.3.14.77, mask /28 (240):
block = 16 → multiples: 0,16,32,48,64,80...
77 → network .64, broadcast .79, hosts .65–.78
```

### Using bc during the defense (allowed)
```bash
echo "obase=2; 172" | bc        # decimal → binary
echo "ibase=2; 10101100" | bc   # binary → decimal
echo "256-240" | bc             # block size
```

### Quick validity checklist before clicking Check
- [ ] No host uses a network or broadcast address
- [ ] No duplicate IPs in a subnet
- [ ] Same mask on all interfaces of one subnet
- [ ] Router interfaces all in different, non-overlapping subnets
- [ ] Every gateway/next-hop is inside the sender's own subnet
- [ ] Return routes exist for every path
- [ ] No private/loopback IPs facing the Internet

---

## 15. Common Errors in the Logs & What They Mean

| Log message (paraphrased) | Cause | Fix |
|---|---|---|
| "destination unreachable" | No route matches, or hosts not in same subnet | Check masks / add route |
| "invalid netmask" | Non-contiguous mask (e.g. 255.0.255.0) | Use a valid mask value |
| "gateway unreachable" / "not on connected network" | Gateway IP not in the host's subnet | Point to the local router interface |
| "is a network/broadcast address" | Host assigned .0 or .255-equivalent of its block | Pick an address strictly inside the range |
| "duplicate IP" | Two interfaces share an IP | Change one |
| "overlaps with..." | Two subnets on a router intersect | Re-plan ranges with block sizes |
| "private address on the Internet" | 10.x / 172.16–31.x / 192.168.x facing the cloud | Use a public address |
| "loopback" | 127.x.x.x used | Never assign loopback to interfaces |
| Packet goes out but never comes back | Missing return route | Add route to the *source* network on the way back |

---

## 16. Appendix: CIDR /24 → /32, the Bit-Value Pattern

### The bit-value table (one octet)

| Bit position | 7   | 6   | 5   | 4   | 3   | 2   | 1   | 0   |
|--------------|-----|-----|-----|-----|-----|-----|-----|-----|
| Value        | 128 | 64  | 32  | 16  | 8   | 4   | 2   | 1   |

Each `/N` doesn't map to *one* bit value — it **accumulates** them from the left.

### /24 is the starting point: last octet = all zeros

```
/24 = 255.255.255.0     → last octet: 00000000
```

The first 24 bits (three full octets of 255) are the network. The whole last octet belongs to hosts.

### Each step past /24 adds the next bit value

Going from /24 to /25 turns on bit 7 (value 128). /26 turns on bit 6 as well (+64). It's a running sum:

| CIDR | Bits turned on in last octet | Sum | Mask octet | Block size |
|------|------------------------------|-----|------------|------------|
| /24  | none                         | 0   | .0         | 256        |
| /25  | 128                          | 128 | .128       | 128        |
| /26  | 128+64                       | 192 | .192       | 64         |
| /27  | 128+64+32                    | 224 | .224       | 32         |
| /28  | 128+64+32+16                 | 240 | .240       | 16         |
| /29  | 128+64+32+16+8               | 248 | .248       | 8          |
| /30  | 128+64+32+16+8+4             | 252 | .252       | 4          |
| /31  | 128+64+32+16+8+4+2           | 254 | .254       | 2          |
| /32  | all eight                    | 255 | .255       | 1          |

The bit-value table is exactly the list of **increments** between consecutive masks:
/27 → /28 adds 16, /29 → /30 adds 4, and so on.

### The complementary view (often faster)

The value of the **last bit turned on** = the **block size** of the subnet:

```
/25 → last bit added is 128 → block 128
/26 → last bit added is  64 → block  64
/27 → last bit added is  32 → block  32
/28 → last bit added is  16 → block  16
/29 → last bit added is   8 → block   8
/30 → last bit added is   4 → block   4
/24 → nothing added → block 256 (the whole octet)
```

Same result as `256 − mask octet`, but read directly off the table.
**One table gives you everything:** mask value (cumulative sum) and block size (last value added).

### Special cases

- **/31** — 2 addresses, 0 usable hosts by the normal rules (network + broadcast eat both). Not used for hosts in NetPractice.
- **/32** — a single address; identifies one exact host, mainly seen in routing tables.

### The pattern repeats in other octets

The same logic applies to the *third* octet for /17 through /23:

| CIDR | Mask            |
|------|-----------------|
| /17  | 255.255.128.0   |
| /18  | 255.255.192.0   |
| /19  | 255.255.224.0   |
| /20  | 255.255.240.0   |
| /21  | 255.255.248.0   |
| /22  | 255.255.252.0   |
| /23  | 255.255.254.0   |

Same sequence (128, 192, 224, 240, 248, 252, 254) — just shifted one octet left. And /9–/15 shift it to the second octet, /1–/7 to the first.

---
 
## 17. Appendix: Switch & Router in ASCII
 
One line to rule them all:
**switch = one network, many machines**
**router = one machine, many networks.**
 
### Switch — builds ONE subnet
 
```
                    ┌──────────────────────────────────┐
                    │  ONE subnet: 192.168.1.0/24      │
                    │                                  │
   Host A ─────────┬┤   all three hosts must have:     │
   192.168.1.11    ││   • same network part (192.168.1)│
                   ││   • same mask (/24)              │
   Host B ─────────┼┤   • different host part          │
   192.168.1.12    ││                                  │
                   ││   the SWITCH itself:             │
   Host C ─────────┴┤   • no IP, no config             │
   192.168.1.13     │   • invisible to the exercise    │
                    └──────────────────────────────────┘
              conceptually: A, B, C are on one big wire
```
 
- Layer 2 device: forwards frames by **MAC address**, learned per port.
- Delivery works via **ARP**: A broadcasts *"who has 192.168.1.12?"* → B replies
  with its MAC → frames then flow directly. (A hub is the dumb layer-1 ancestor:
  it repeats every signal to all ports, no learning — not used in NetPractice.)
- **NetPractice rule:** a switch means *"every interface plugged into me must be
  in the same subnet."* Nothing to configure on the switch itself, ever.

### Router — stands BETWEEN subnets
 
```
      subnet 1: 192.168.1.0/24              subnet 2: 10.0.0.0/30
   ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐        ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐
 
   Host A ──[switch]──── R1-iface1 ══╗   ╔══ R1-iface2 ────────── R2 ...
   192.168.1.11          192.168.1.1 ║   ║   10.0.0.1
   gw: 192.168.1.1                   ║   ║
                                   ┌─╨───╨─┐
   └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘   │ROUTER │  └ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘
                                   │  R1   │
                                   └───────┘
        the router is the ONLY thing that exists in both subnets;
        each interface obeys the rules of the subnet it stands in
```




---

`echo "ibase=2; 101101" | bc`
`echo "obase=2; 45" | bc`


