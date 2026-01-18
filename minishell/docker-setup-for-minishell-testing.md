# Docker Setup for Minishell Testing

## Why Use Docker?

When you're away from campus, you need to ensure your minishell works in an environment that matches the school's computers. Docker lets you replicate the exact system specs (OS, compiler versions, tools)


## Check Computer Specs in the Campus

Run these commands on a campus machine:

```
# Operating system and version
cat /etc/os-release

# Kernel version
uname -r

# GCC version
gcc --version

# Available tools
which valgrind make bash
valgrind --version
make --version
bash --version

# File descriptor limits
ulimit -n
```

## Install Docker (Arch Linux)

```
# Install Docker
sudo pacman -S docker

# Start Docker service
sudo systemctl start docker
sudo systemctl enable docker

# Add your user to docker group (avoid sudo)
sudo usermod -aG docker $USER

# Log out and back in, then verify
docker --version
```


## Create Dockerfile

Create a file named `Dockerfile` in your minishell project root:

```
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install tools matching school environment
RUN apt-get update && apt-get install -y \
    bash \
    gcc-10 \
    g++-10 \
    make \
    valgrind \
    libreadline-dev \
    coreutils \
    grep \
    && rm -rf /var/lib/apt/lists/*

# Set GCC 10 as default compiler
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 100

# Create 'cc' symlink (many Makefiles use 'cc' instead of 'gcc')
RUN ln -s /usr/bin/gcc /usr/bin/cc

# Use bash as default shell
RUN ln -sf /bin/bash /bin/sh

# Set file descriptor limit
# This limits how many files can be open at once
RUN echo "* soft nofile 1024" >> /etc/security/limits.conf && \
    echo "* hard nofile 1024" >> /etc/security/limits.conf

SHELL ["/bin/bash", "-c"]

# Create regular user (not root)
RUN useradd -m -s /bin/bash student
USER student
WORKDIR /home/student

CMD ["/bin/bash"]
```


## Understanding the Dockerfile

Key parts explained:

**FROM ubuntu:22.04**
- Base image (match this to school's OS version)

**gcc-10, g++-10**
- Specific compiler version (match school's gcc version)

**update-alternatives**
- Makes gcc-10 the default when you type `gcc`

**cc symlink**
- Many Makefiles use `cc` command instead of `gcc`
- This makes both work

**File descriptor limit**
- Controls max open files (stdin/out/err, opened files, pipes, etc.)
- 1024 is an example, it can be less or more

**Regular user (student)**
- Don't run as root for security
- Matches how you work on school computers


## Build and Run

### Build the image (one time)

```
cd /path/to/your/minishell
docker build -t minishell .
```

This creates an image named `minishell` from your Dockerfile.


### Enter the container

```
docker run -it --rm -v $(pwd):/home/student/minishell -w /home/student/minishell minishell
```

Breakdown:
- `-it`: Interactive terminal
- `--rm`: Auto-delete container when you exit
- `-v $(pwd):/home/student/minishell`: Mount your project inside container
- `-w /home/student/minishell`: Start in project directory
- `minishell`: Use the image you built


### Inside the container

You're now in a bash shell that matches school specs. Work normally:

```
# Build your project
make

# Run minishell
./minishell

# Test with valgrind
valgrind ./minishell
```


### Exit the container

```
exit
```

The container is deleted automatically (because of `--rm` flag).
Your files are safe - they're in your host machine, just mounted in container.
