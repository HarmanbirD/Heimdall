# Heimdall — File Integrity Monitoring Daemon

**Heimdall** is a C-based File Integrity Monitoring (FIM) daemon for Linux systems.  
It continuously verifies the integrity of critical files and directories using **SHA-256 hashing** (via OpenSSL’s EVP API), logs results to both **syslog** and `/var/log/heimdall.log`, and supports multiple hashing levels — **directory**, **file**, and **line**.

---

##  Features

-  **Systemd-compatible daemon**
-  **SHA-256 hashing** of files, directories, and per-line contents
-  **Recursive directory traversal**
-  **Configurable monitoring** via `/etc/heimdall.conf`

---

##  Configuration

### Default Monitored Paths

Heimdall monitors several critical Linux configuration files by default:

- /etc/shadow  
- /etc/passwd  
- /etc/group  
- /etc/sudoers  
- /etc/ssh/sshd_config  
- /etc/ssh/ssh_config  
- /etc/pam.d/  
- /etc/hosts  
- /etc/resolv.conf

### User Configuration (`/etc/heimdall.conf`)
You can extend the default paths by creating `/etc/heimdall.conf`.

Each line uses a CSV-like format:

```
<path>, <level>, <alert>
```

Where:

- `<level>` = `d` (directory), `f` (file), or `l` (line)  
- `<alert>` = `r` (red), `y` (yellow), or `g` (green)

**Example:**
```
/home/name/Documents/test/, d, y
/etc/httpd/conf/httpd.conf, f, r
```

## Building from Source

### Build Steps

```bash
cd Heimdall
mkdir build && cd build
cmake ..
make
sudo make install
```

This installs the binary to:
```
sudo /usr/local/bin/Heimdall
```

## Running Heimdall

### Run as a Systemd Service (Recommended)

```
sudo systemctl daemon-reload
sudo systemctl enable heimdall.service
sudo systemctl start heimdall.service
sudo systemctl status heimdall.service
```

To view logs:
```
sudo journalctl -u heimdall.service -f
```

### Start Manually

```
sudo /usr/local/bin/Heimdall
```

## Log Files

- System log: View via `journalctl -t Heimdall`
- Local log: `/var/log/heimdall.log`
