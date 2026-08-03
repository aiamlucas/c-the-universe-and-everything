# IP Address Types — Public, Private, Reserved, Multicast & Anycast

A reference companion to the NetPractice study guides — this one's about the
*kinds* of addresses that exist, not the subnetting math.

---

## Table of Contents

1. [Public vs Private IP Addresses](#1-public-vs-private-ip-addresses)
2. [Private Ranges (RFC 1918)](#2-private-ranges-rfc-1918)
3. [Reserved & Special-Purpose Ranges](#3-reserved--special-purpose-ranges)
4. [Unicast, Broadcast, Multicast & Anycast](#4-unicast-broadcast-multicast--anycast)
5. [Multicast in Detail](#5-multicast-in-detail)
6. [Anycast in Detail](#6-anycast-in-detail)
7. [Quick Reference Table](#7-quick-reference-table)

---

## 1. Public vs Private IP Addresses

```
PUBLIC IP            → globally unique, reachable directly on the Internet
                        (like a street address the whole world can mail to)

PRIVATE IP            → only unique WITHIN one local network, never routed
                        on the public Internet (like an apartment number —
                        meaningless without the building's street address)
```

Every device with a private IP that needs to reach the Internet does so
through **NAT** (Network Address Translation) — the router rewrites the
private source address to its one public address on the way out, and
translates replies back on the way in.

```
  PC 192.168.1.10 ──┐
  PC 192.168.1.11 ──┼──[ROUTER + NAT]────── ISP's public IP ── Internet
  PC 192.168.1.12 ──┘      82.64.7.9

  This is why millions of homes can all use "192.168.1.x" internally
  without ever colliding — none of it is visible outside the router.
```

---

## 2. Private Ranges (RFC 1918)

The three ranges everyone reuses, standardized in 1996 (RFC 1918):

| Range                             | CIDR             | Size                 | Typical use                                |
|-----------------------------------|------------------|----------------------|---------------------------------------------|
| `10.0.0.0` – `10.255.255.255`     | `10.0.0.0/8`     | 16,777,216 addresses | Large organizations, big internal networks |
| `172.16.0.0` – `172.31.255.255`   | `172.16.0.0/12`  | 1,048,576 addresses  | Medium-sized networks                      |
| `192.168.0.0` – `192.168.255.255` | `192.168.0.0/16` | 65,536 addresses     | Home networks, small offices               |

```
A: 10.0.0.0/8       ██──────────────────────  1 network   × 16.7M addresses
B: 172.16.0.0/12    ████────────────────────  16 networks × 65,536 addresses
C: 192.168.0.0/16   ████████────────────────  256 networks × 256 addresses
```

**Trivia:** these boundaries are fossils of the old class A/B/C system
(pre-1993). And despite how universal `192.168.x.x` feels today, there was
no deep engineering reason it was chosen over any other block — RFC 1918
mostly formalized address ranges people were *already* using informally;
router manufacturers later converged on `192.168.0.1`/`192.168.1.1` as
factory defaults simply because everyone else was doing it.

---

## 3. Reserved & Special-Purpose Ranges

Beyond the three private ranges, IPv4 sets aside several other blocks for
specific jobs — never assigned to an ordinary host on the open Internet:

| Range                                                | Name                            | Purpose                                                                                               |
|------------------------------------------------------|---------------------------------|-------------------------------------------------------------------------------------------------------|
| `0.0.0.0/8`                                          | "This network"                  | Used as a source address meaning "I don't have an address yet" (e.g. during DHCP discovery)           |
| `100.64.0.0/10`                                      | Carrier-Grade NAT (CGNAT)       | ISPs use this internally when NAT-ing many customers behind one public IP                             |
| `127.0.0.0/8`                                        | Loopback                        | "This machine, talking to itself" — never leaves the device                                           |
| `169.254.0.0/16`                                     | Link-local (APIPA)              | A device self-assigns this when it can't reach a DHCP server — a diagnostic red flag in real networks |
| `192.0.0.0/24`                                       | IETF protocol assignments       | Reserved for protocol development                                                                     |
| `192.0.2.0/24`, `198.51.100.0/24`, `203.0.113.0/24`  | Documentation (TEST-NET 1/2/3)  | Used in manuals, RFCs, and examples — never assigned to real devices                                  |
| `198.18.0.0/15`                                      | Benchmarking                    | Reserved for network equipment testing                                                                |
| `224.0.0.0/4`                                        | Multicast                       | "Send to a whole interested group," not one host (see §5)                                             |
| `240.0.0.0/4`                                        | Reserved (old "Class E")        | Reserved for future use, never put into general service                                               |
| `255.255.255.255`                                    | Limited broadcast               | "Everyone on this local wire" — never routed past your own subnet                                     |

**Why `169.254.x.x` matters practically:** if a real device on your network
suddenly shows an address in this range, it's the OS itself telling you
"I tried to get an IP and failed" — a genuinely useful diagnostic signal,
not just a rule to memorize.

---

## 4. Unicast, Broadcast, Multicast & Anycast

Four different delivery models for "who receives this packet":

```
UNICAST                     one sender  →  ONE specific receiver
                             (the default — almost everything you do)

  A ──────────► B

BROADCAST                    one sender  →  EVERY device in the subnet
                             (e.g. DHCP discovery, ARP requests)

  A ──────────► B
    ├──────────► C
    └──────────► D

MULTICAST                    one sender  →  only devices that JOINED a group
                             (e.g. streaming to subscribed viewers)

  A ──────────► B (joined)
    ╌╌╌╌╌╌╌╌╌╌╌╳ C (not joined — doesn't receive)
    ──────────► D (joined)

ANYCAST                      one address →  routed to the NEAREST of many
                             identical servers advertising it
                             (e.g. DNS root servers, CDNs)

  A ──────────► [nearest server advertising this address]
                (which physical server answers depends on A's location)
```

---

## 5. Multicast in Detail

**Range:** `224.0.0.0` – `239.255.255.255` (`224.0.0.0/4`)

Multicast lets one sender reach many receivers *efficiently* — the network
only duplicates the packet at the points where paths actually diverge,
rather than the sender blasting a separate copy to every recipient.

```
Sender ──► [Router] ──┬──► Group member 1
                       ├──► Group member 2
                       └──► (no copy sent where no member exists)
```

Devices **join** a multicast group (using IGMP, Internet Group Management
Protocol) to start receiving traffic sent to that group's address; nobody
else on the network sees it. Common real-world uses: IPTV/video streaming,
some routing protocols (OSPF uses `224.0.0.5`/`224.0.0.6` to talk to
neighbors), and service discovery protocols.

```
224.0.0.0/24     reserved for local network control traffic (e.g. routing)
239.0.0.0/8      "administratively scoped" — like a private range, but for multicast
```

---

## 6. Anycast in Detail

Anycast isn't a separate address *range* — it's a **routing technique**:
the *same* IP address is advertised from multiple physical locations, and
the network's normal routing automatically sends each client to whichever
one is closest (in terms of network path, not necessarily geography).

```
        Anycast address: 1.1.1.1

  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐
  │  Server in   │    │  Server in   │    │  Server in   │
  │  New York    │    │  London      │    │  Tokyo       │
  └──────┬───────┘    └───────┬──────┘    └────────┬─────┘
         │                    │                    │
         └────────────────────┼────────────────────┘
                     all three answer to
                     the SAME IP address

  User in Paris  → routed to London (nearest by network path)
  User in Osaka  → routed to Tokyo  (nearest by network path)
  Same address, different physical server, depending on the client!
```

**Real-world examples:**
- **DNS root servers** — there are only 13 *named* root servers, but
  hundreds of physical machines worldwide answer via anycast.
- **CDNs** (Cloudflare, Google Public DNS `8.8.8.8`) — the same public IP
  is announced from data centers all over the world; you're automatically
  routed to a nearby one without any special configuration on your end.

**Why it's useful:** lower latency (nearest server answers), automatic
failover (if one location goes down, routing simply shifts to the next
nearest), and no need for the client to know or care which physical
machine actually replied.

---

## 7. Quick Reference Table

```
Public                → globally unique, routable on the real Internet
Private (10/8)        → large orgs, 16.7M addresses
Private (172.16/12)   → medium networks, 1M addresses
Private (192.168/16)  → home/small office, 65,536 addresses
Loopback (127/8)      → "myself," never leaves the device
Link-local (169.254)  → self-assigned when DHCP fails — a red flag
CGNAT (100.64/10)     → ISP-internal NAT space
Multicast (224/4)     → one-to-many, only group members receive
Reserved (240/4)      → unused "Class E," future reserve
Broadcast (255.255.255.255) → everyone on the local subnet, no further
Anycast               → not a range — same address, nearest server answers
```
