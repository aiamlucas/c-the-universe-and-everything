# Born to Be Root

---

## Chapter 1: Foundations of Virtualization, Linux, and System Access

---

## Why This Chapter?

Before diving into encryption, user policies, or sudo logs, it’s essential to understand what environment you're working in and how it operates. This chapter provides the system-level context for everything that follows in the Born to Be Root project.

---

## Virtualization 101

**Virtualization** allows you to run a full operating system (called a *guest*) inside another system (called the *host*) as if it were a physical machine.

### Key Concepts

- **Hypervisor**: Software that enables virtualization. Two types:
  - Type 1: Bare metal (runs directly on hardware)
  - Type 2: Hosted (runs within an OS — like VirtualBox or UTM)
- **VM (Virtual Machine)**: A self-contained OS instance.
- **Disk Image**: A file that represents a virtual hard drive (e.g., ```.vdi``` or ```.qcow2```)
- **ISO**: A bootable image of a CD/DVD used to install the OS.

### Why Use It?

- Isolate your experiments and configs
- Easily reset or duplicate environments
- Mimic real-world server setups

---

## The Linux Operating System

This project asks you to choose between **Debian** and **Rocky Linux**. Both are Unix-like systems, but they differ in philosophy and tooling.

### Debian (Recommended)

- Community-driven
- Package manager: `apt` / `aptitude`
- Uses **AppArmor** for security

### Rocky Linux

- Enterprise-focused (RHEL-compatible)
- Package manager: `dnf` (replaces `yum`)
- Uses **SELinux** for security

### Package Management Comparison

| Tool        | Debian                     | Rocky Linux              |
|-------------|----------------------------|--------------------------|
| Binary mgr  | `apt` / `apt-get` / `dpkg` | `dnf` / `rpm`            |
| GUI options | `aptitude`                 | `dnfdragora` (optional)  |
| Security    | `AppArmor`                 | `SELinux`                |

Knowing how your OS handles updates, installs, and service control is critical for long-term maintainability and security.

---

## Core Concepts in Linux Administration

### Systemd and Services

Both Debian and Rocky use **systemd** to manage services:
- Start/stop services: ```systemctl start ssh```
- Enable on boot: ```systemctl enable ssh```

Useful systemd commands:
- ```systemctl status``` — list service states
- ```systemctl list-units --type=service```

### Users and Groups

- Each user has a UID; each group has a GID
- You can add users with ```adduser``` or ```useradd```
- Modify group memberships with ```usermod -aG```

### Root and Sudo

- **Root** is the superuser; it can do anything.
- Regular users elevate privileges using ```sudo```.
- **Sudo policies** (who can use it, how, and what gets logged) are key to secure system design.

---

## Encrypted Partitions and LVM (Intro)

This project requires you to create **at least two encrypted partitions using LVM**.

### What Is LVM?

LVM (Logical Volume Manager) is a layer between physical storage and logical partitions.

Benefits:
- Resize partitions without rebooting
- Combine multiple disks into one logical unit
- Snapshots and backups are easier

### What Is Disk Encryption?

Encryption secures data on your disk using cryptographic algorithms.  
Even if someone steals the disk, they can’t read its contents without the key.

In Linux, this is typically handled using **LUKS (Linux Unified Key Setup)**.

---

## SSH and Network Access

SSH (Secure Shell) allows remote access to your server.  
For this project, SSH must run on port **4242** and must **deny root login**.

### How SSH Works

- Server runs ```sshd```
- Client connects using ```ssh user@ip -p 4242```
- Login is authenticated by password or SSH key

### Key Configuration File

```/etc/ssh/sshd_config```

Important options:
- ```Port 4242```
- ```PermitRootLogin no```
- ```PasswordAuthentication yes``` (or no, if using keys)

Understanding SSH is essential to managing any real-world server.

---

## Firewall Fundamentals

You must configure a firewall and open **only** port 4242.

### For Debian: UFW (Uncomplicated Firewall)

Basic commands:
- ```ufw enable```
- ```ufw allow 4242/tcp```
- ```ufw status verbose```

### For Rocky: firewalld

- Uses zones to manage rules
- ```firewall-cmd --add-port=4242/tcp --permanent```
- ```firewall-cmd --reload```

---

## Summary

Before doing anything technical in this project, you must:
- Understand what virtualization is and how to use it properly
- Choose between Debian or Rocky Linux (knowing their core differences)
- Get comfortable with the Linux CLI, systemd, package managers, users, and groups
- Know the role of root, how sudo elevates access, and how SSH provides remote control
- Be aware of firewalls and how to limit your exposed services

This foundational theory prepares you for the next chapter — **Hardening the System and Managing Users**.

---

## Chapter 2: System Hardening and User Management

---

## Why System Hardening?

**System hardening** means reducing potential vulnerabilities in your system.  
This involves securing users, passwords, permissions, access points, and services — everything that could be exploited.

This chapter explains the core theory behind each element you’re expected to configure.

---

## Hostname and Identity

Your server’s hostname identifies it on a network and in logs.  
This project requires it to match your login and end with ```42``` (e.g., ```wil42```).

Changing the hostname requires understanding:
- ```/etc/hostname``` → contains the system’s hostname
- ```/etc/hosts``` → maps hostname to IP (for local resolution)

---

## User Management

You must create:
- A root account (already exists)
- A second user (your login)
- Assign this user to groups: ```user42``` and ```sudo```

### Key Concepts

- ```adduser``` creates a user and sets up the home directory
- ```usermod -aG group user``` adds a user to a group
- ```groups username``` shows which groups a user belongs to

### Why Groups?

Groups define what resources a user can access or manage.  
The ```sudo``` group is critical — only members of this group can execute privileged commands.

---

## Sudo Policy and Logging

Sudo allows non-root users to execute commands as root — but it must be controlled.

### Configuration File

- ```/etc/sudoers```
- Use ```visudo``` to edit it safely (it checks for syntax errors)

### What You Need to Understand

- **Authentication attempts**: You must limit them to 3
- **Custom error messages**: Displayed after a failed attempt
- **Logging**: All sudo commands must be archived
- **TTY requirement**: Prevents background execution of privileged commands
- **PATH restriction**: Limits what commands can be run with sudo

#### Example log directory:
```/var/log/sudo/```

Logs should include:
- The command executed
- The timestamp
- The user who ran it

---

## Password Policy and Security

This is one of the most critical parts of the project.

### Requirements You Need to Understand

| Policy                             | Description                                                                 |
|------------------------------------|-----------------------------------------------------------------------------|
| Expiry                             | Password must expire every 30 days                                          |
| Minimum usage period               | Cannot be changed again within 2 days                                       |
| Expiry warning                     | Users must be warned 7 days before expiration                               |
| Complexity                         | Must include uppercase, lowercase, numbers                                 |
| Length                             | Minimum 10 characters                                                       |
| Repetition                         | No more than 3 identical characters in a row                                |
| No username                        | Cannot contain the user's name                                              |
| Reuse restriction (partial)        | For non-root, at least 7 characters different from old password             |

### Tools and Concepts

- ```chage```: Changes account aging settings
- ```pam_pwquality.so```: PAM module used to enforce password strength
- ```/etc/login.defs```: Defines global login policies

---

## SELinux and AppArmor (MACs)

These are **Mandatory Access Control** (MAC) frameworks used for extra security.

### AppArmor (Debian)

- Uses profiles to confine what programs can access
- Easier to configure than SELinux
- Profiles stored in ```/etc/apparmor.d/```
- Must be **enabled and running** at boot

### SELinux (Rocky)

- More powerful and complex than AppArmor
- Uses **contexts** to enforce access rights
- Controlled using ```sestatus```, ```getenforce```, ```setenforce```
- Modes: ```enforcing```, ```permissive```, ```disabled```

### You Must Understand

- What SELinux or AppArmor is doing
- Why it’s different from standard permissions (DAC)
- What it means to confine processes and users
- How to check if it's active and correctly configured

---

## SSH Best Practices

- Use port **4242** instead of the default 22
- **Do not allow root login**
- SSH config file: ```/etc/ssh/sshd_config```

Key directives:
- ```Port 4242```
- ```PermitRootLogin no```
- ```PasswordAuthentication yes/no```

---

## Firewall Rules

Only one port should be open: **4242**

### Debian: UFW (Uncomplicated Firewall)

- ```ufw enable```
- ```ufw allow 4242/tcp```
- ```ufw default deny incoming```

### Rocky: firewalld

- ```firewall-cmd --add-port=4242/tcp --permanent```
- ```firewall-cmd --reload```

Firewalls are your **first layer of defense**. Misconfigured firewalls can leave your system wide open.

---

## Summary

This chapter prepared you for the hardening tasks in Born to Be Root. You now understand:

- How to manage users and groups
- Why password policies matter and how they’re enforced
- How sudo works and how it must be secured and audited
- What firewalls do and how to restrict ports
- Why AppArmor/SELinux offer additional protection beyond file permissions
- Why SSH access must be limited and monitored

In the next chapter, we’ll look at **monitoring, automation, and bonus-level system setup**.

---

## Chapter 3: Monitoring, Automation, and Extras

---

## Why Monitoring Matters

Monitoring is essential in system administration because it helps you:
- Know if your system is under heavy load
- Detect failures or performance issues early
- Keep track of user activity and network behavior
- Gather useful data for auditing and optimization

In this project, you write a ```monitoring.sh``` script that prints real-time system stats — it's your first real taste of automated server observation.

---

## System Monitoring Metrics

Your monitoring script must collect and print various system metrics. Here's what each one means:

---

### Architecture and Kernel

**Command:**
```uname -a```

This outputs:
- Kernel name
- Hostname
- Kernel release and version
- Architecture (e.g., x86_64)

---

### Physical and Virtual CPUs

**Commands:**
- ```lscpu```
- ```grep "CPU(s):" /proc/cpuinfo```

**Terms:**
- **Physical CPU**: A physical core on the chip
- **vCPU**: Virtual CPU (logical thread), seen in virtualization or hyper-threading

---

### RAM Usage

**Command:**
```free -m```

Output includes:
- Total, used, and available memory
- Use this to calculate a usage percentage

---

### Disk Usage

**Command:**
```df -h /```

Shows space used on the root filesystem. You'll display:
- Total disk
- Used disk
- Percentage used

---

### CPU Load

**Command:**
```top -bn1 | grep "Cpu(s)"```

Displays the current CPU utilization. Look for the user/system/idle split to determine total usage.

---

### Last Boot

**Command:**
```who -b```

Gives the last system boot time — useful for uptime tracking.

---

### LVM Status

**Command:**
```lsblk | grep lvm```

You just need to check whether LVM volumes are present and active.

---

### Active TCP Connections

**Command:**
```ss -ta | grep ESTAB | wc -l```

Counts how many TCP connections are currently established.

---

### Logged-in Users

**Command:**
```who | wc -l```

Counts how many users are currently logged into the system.

---

### Network Information

**Commands:**
- IP: ```hostname -I```
- MAC: ```ip link show```

The MAC address is tied to your virtual network interface.

---

### Sudo Usage Count

**Command:**
```journalctl _COMM=sudo | grep COMMAND | wc -l```

This counts all sudo commands executed and logged.

---

## Automation with Cron

Cron is a time-based job scheduler.  
It runs tasks automatically based on defined intervals.

---

### How It Works

Cron reads from:
- ```/etc/crontab```
- ```/etc/cron.d/*```
- User-specific crontabs via ```crontab -e```

Each line defines:
```
# ┌──────────── minute (0 - 59)
# │ ┌────────── hour (0 - 23)
# │ │ ┌──────── day of month (1 - 31)
# │ │ │ ┌────── month (1 - 12)
# │ │ │ │ ┌──── day of week (0 - 7) (Sunday = 0 or 7)
# │ │ │ │ │
# │ │ │ │ │
# * * * * * command to execute
```

### Your Use Case

To run your monitoring script every 10 minutes:
```
*/10 * * * * root bash /path/to/monitoring.sh
```

Make sure output appears on all logged-in users’ terminals:

**Tool:**
```wall``` — broadcast messages to all terminal sessions

---

## Writing Bash Scripts

Your ```monitoring.sh``` script is written in Bash — a Unix shell and scripting language.

### Bash Concepts You Should Know

- Variables: ```name=value```
- Arithmetic: ```$(( expression ))```
- Loops: ```for```, ```while```
- Conditionals: ```if [[ condition ]]; then```
- Command substitution: ```$(command)```
- Output formatting: ```echo```, ```printf```

### Wall Example

To send to all users:

```
wall "Server is at 70% CPU usage"
```

This sends a broadcast message to all terminal users.

---

## Bonus Topics Overview

If you reach the bonus stage, you can expand your system with extra services.  
The most common example is setting up a **WordPress site** on your server.

---

### Typical Bonus Stack

- **Lighttpd**: Lightweight web server
- **MariaDB**: MySQL-compatible database
- **PHP**: Scripting language for dynamic web pages

This teaches you how real web applications are deployed.

---

### Other Possible Services

You can add one additional service (NGINX and Apache2 are excluded).  
Choose something useful and be ready to explain why you added it.

**Examples:**
- File sharing server (e.g., Samba)
- VPN server
- Mail server
- Static file server via Python HTTP module

---

### Security Reminder

If you add extra services:
- You **must open** extra ports in UFW/firewalld
- You **must justify** your configuration choices

---

## Summary

This chapter covered:
- What to monitor and how to calculate it
- Key Bash tools and scripting strategies
- Cron and timed automation basics
- Bonus service considerations and examples

---

By now, you’ve explored every major area of system configuration, security, and monitoring that Born to Be Root expects.  
While the implementation is up to you, this guide should give you the understanding to do it confidently.
