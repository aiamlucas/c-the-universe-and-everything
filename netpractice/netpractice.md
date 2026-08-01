# NetPractice

## 0. Concepts 

| Term                  | One-line meaning                                                                                                   |
|-----------------------|--------------------------------------------------------------------------------------------------------------------|
| **IP address**        | A device's address on a network — 32 bits, written as 4 decimal octets (`192.168.1.10`)                            |
| **Subnet mask**       | Says where the IP splits into *network part* (must match to talk directly) and *host part* (the individual device) |
| **CIDR (`/N`)**       | Shorthand for the mask: N = how many leading bits are network bits                                                 |
| **Network address**   | The first address of a subnet — names the subnet itself, never assignable to a device                              |
| **Broadcast address** | The last address of a subnet — reaches every device in it at once, never assignable                                |
| **Host**              | Any device with its own IP + mask (a computer, a router interface)                                                 |
| **Gateway**           | The router's address *inside your own subnet* — your door to everything outside it                                 |
| **Router**            | Connects *different* subnets; each of its interfaces lives in its own subnet                                       |
| **Switch**            | Connects devices *within* one subnet; no IP, no configuration                                                      |
| **Route**             | A rule: `destination → next hop` — "packets to X, hand to Y"                                                       |


---

## 1. The one picture: an IP address is two numbers glued together

```
        192.168.1        .10
        ─────────────    ───
        NETWORK part     HOST part
        "which street"   "which house"

        WHERE the glue point sits = the subnet mask.
```

---

## 2. The reference table

Every legal mask, in every position, with everything you'd ever compute from
it. This single table replaces almost all binary math during the defense.

```
CIDR   octet 1   octet 2   octet 3   octet 4     dotted mask           power of 2   addresses
/32    11111111  11111111  11111111  11111111    255.255.255.255          2^0             1
/31    11111111  11111111  11111111  11111110    255.255.255.254          2^1             2
/30    11111111  11111111  11111111  11111100    255.255.255.252          2^2             4
/29    11111111  11111111  11111111  11111000    255.255.255.248          2^3             8
/28    11111111  11111111  11111111  11110000    255.255.255.240          2^4            16
/27    11111111  11111111  11111111  11100000    255.255.255.224          2^5            32
/26    11111111  11111111  11111111  11000000    255.255.255.192          2^6            64
/25    11111111  11111111  11111111  10000000    255.255.255.128          2^7           128
/24    11111111  11111111  11111111  00000000    255.255.255.  0          2^8           256
                           ─────────────────────────────────────  ← octet 4 now ALL host
/23    11111111  11111111  11111110  00000000    255.255.254.  0          2^9           512
/22    11111111  11111111  11111100  00000000    255.255.252.  0          2^10         1024
/21    11111111  11111111  11111000  00000000    255.255.248.  0          2^11         2048
/20    11111111  11111111  11110000  00000000    255.255.240.  0          2^12         4096
/19    11111111  11111111  11100000  00000000    255.255.224.  0          2^13         8192
/18    11111111  11111111  11000000  00000000    255.255.192.  0          2^14        16384
/17    11111111  11111111  10000000  00000000    255.255.128.  0          2^15        32768
/16    11111111  11111111  00000000  00000000    255.255.  0.  0          2^16        65536
                          ─────────────────────────────────────  ← octet 3 now ALL host
...
```
---

## 3. Core mental models

Four small pictures. Once these click, every level is the same puzzle at a
different scale.

### Model A — the checkpoint road (what `IP % block` computes)

Multiples of the block are checkpoints; checkpoints ARE network addresses.
`IP % block` = "how far past the last checkpoint am I?" Subtract it → you're
standing on your network address.

```
block = 32:
   0     32    64    96    128   160   192   224   256
   ├─────┼─────┼─────┼─────┼─────┼─────┼──█──┼─────┤
                                          ↑
                                    IP = 222
   222 % 32 = 30  (walked 30 past checkpoint 192)
   222 − 30 = 192 = network address
```

### Model B — the full landscape (all subnets of a mask, at once)

Mask `.224` / block 32 chops one octet into 8 independent networks. Every
box: first = network, last = broadcast, different boxes need a router.

```
┌─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┐
│  .0–.31 │ .32–.63 │ .64–.95 │ .96–.127│.128–.159│.160–.191│.192–.223│.224–.255│
│         │         │         │         │         │         │   ███   │         │
└─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┘
  net .0    net .32   net .64   net .96   net .128  net .160  net .192  net .224
  bc  .31   bc  .63   bc  .95   bc  .127  bc  .159  bc  .191  bc  .223  bc  .255
                                          222 lives here ──────────┘
```

Zoomed into the one box that matters:

```
        192.168.87.192/27
   ┌──────┬────────────────────────────────────┬──────┬─────┐
   │ .192 │ .193  .194   ……   .220  .221       │ .222 │ .223│
   │ NET  │      free hosts (29 choices)       │taken │BCAST│
   └──────┴────────────────────────────────────┴──────┴─────┘
      ✗                  ✓                        ✗      ✗
```

### Model C — anatomy of any block (universal shape)

Every block, at every mask, has exactly this shape:

```
        ┌──────────────────────────────────────────────┐
        │ NETWORK  │      hosts (usable)     │BROADCAST│
        │ (first)  │                         │ (last)  │
        └──────────────────────────────────────────────┘
          forbidden          usable             forbidden
```

### Model D — the mask as a stencil (what `IP & mask` computes)

Where the mask has 1 → keep the IP's bit. Where 0 → erase it.

```
IP:    222 = 1 1 0 1 1 1 1 0
mask:  224 = 1 1 1 0 0 0 0 0
             │ │ │ ┌─erased─┐
AND:         1 1 0 0 0 0 0 0  = 192  ← the network address
```

**`IP − (IP % block)` and `IP & mask` are the same operation in two
costumes.** bc has no `&`, so we use the modulo costume — same answer either
way.

---

## 4. The four formulas (and how to run them in `bc`)

```
① block     = 256 − mask_octet          ← size of each subnet
② network   = IP − (IP % block)         ← land on the checkpoint behind you
③ broadcast = network + block − 1       ← last address of the block
④ hosts     = network+1 … broadcast−1   ← minus any duplicates
```

---

## 5. Devices — switch vs. router

```
SWITCH                              ROUTER
┌─────────────────────┐             ┌─────────────────────┐
│   PC1 ─┐            │             │   PC1 ──[iface 1]   │
│   PC2 ─┼─[SWITCH]   │             │                     │
│   PC3 ─┘            │             │   PC2 ──[iface 2]   │
│  ONE shared subnet  │             │  iface 1 and 2 are  │
│  no IP, no config   │             │  TWO DIFFERENT      │
│                     │             │  subnets            │
└─────────────────────┘             └─────────────────────┘
```

```
SWITCH = one network, many machines   (everyone must share a subnet)
ROUTER = one machine, many networks   (every interface is its own subnet)
```

Delivery on the wire uses **ARP**: *"who has 192.168.1.20? tell me your
MAC"* → owner replies → frames flow directly from then on. Only matters on
the local switch/wire; not simulated in NetPractice, but it's what "directly
connected" means physically.

---

## 6. Gateway & routing tables

A **route** is one rule: `destination → next hop`.

```
① destination = default (0.0.0.0/0)     ← "everything outside my subnet"
② next hop    = the router interface INSIDE MY OWN SUBNET   ← the gateway
```

```
Machine A  29.15.134.x/25                Machine B  148.24.x.x/18
route: default => 29.15.134.126          route: default => 148.24.241.254
                  R1 (A's own door) ✓                     R2 (B's own door) ✓

✗ pointing at the OTHER side's router interface — not reachable directly
✗ pointing at an address in nobody's subnet
```

A router needs its own route entries only for networks **beyond** another
router — a directly-attached subnet is already known automatically. Later
levels: routes are needed in **both directions** (forward AND return), and
`default` = `0.0.0.0/0` always.

---

## 7. Private ranges, NAT, DHCP/DNS, loopback — quick table

| Concept | What it is | Why NetPractice cares |
|---|---|---|
| **Private ranges** | `10.0.0.0/8`, `172.16.0.0/12`, `192.168.0.0/16` — reusable, never on the public Internet | Interfaces facing the Internet cloud must NOT use these |
| **NAT** | Router rewrites private→public address so a whole LAN shares one public IP | Simulation has no NAT — public-facing fields need genuinely public IPs |
| **Loopback** | `127.0.0.0/8` — "this machine talking to itself," 16.7M addresses, never on the wire | Never assign `127.x.x.x` to any interface |
| **DHCP** | Auto-hands out IP/mask/gateway/DNS when a device joins | In NetPractice, *you* are the DHCP — filling fields by hand |
| **DNS** | Translates names → IPs (`google.com → a.b.c.d`) | Not simulated; just context for "how real networks find each other" |

Private-range sizes, visually:

```
A: 10.0.0.0/8       ██──────────────────────  1 network   × 16.7M addresses
B: 172.16.0.0/12    ████────────────────────  16 networks × 65,536 addresses
C: 192.168.0.0/16   ████████────────────────  256 networks × 256 addresses
```

---

## 8. VLSM — different subnets, different sizes

Real designs (and NetPractice level 7) rarely want equal-sized subnets. VLSM
= carve the **biggest** required piece first, then keep cutting the
remainder into progressively smaller pieces:

```
┌─────────────────────────────────────────────────────────────┐
│                    10.0.0.0/24  (256 addr)                  │
├───────────────────────────────┬───────────────┬───┬───┬───┬─┤
│      LAN 1 needs 100 hosts    │  LAN 2: 50    │/30│/30│/30│…│
│         → /25 (126 usable)    │  → /26 (62)   │ 2 │ 2 │ 2 │  │
└───────────────────────────────┴───────────────┴───┴───┴───┴─┘
        biggest chunk first          next size        router links last
```

---
