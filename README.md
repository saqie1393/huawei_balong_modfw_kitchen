# Huawei Balong ModFW Kitchen (Automated Builder)

This repository provides an **automated GitHub Actions workflow** to build, modify, and sign custom firmware for Huawei Balong modems and routers. 

Instead of manually extracting files, running scripts, and signing the firmware on your local machine, you can now trigger a build directly from GitHub and download the ready-to-flash custom firmware.

## 🚀 Features

* **Automated Build:** Extracts original firmware, applies modifications, repacks partitions, and signs the final firmware.
* **GitHub Releases:** The final signed firmware is automatically uploaded to GitHub Releases for easy downloading.
* **Universal Support:** Select your Balong family, device overlay, and firmware version directly from dropdown menus.
* **Signed & Ready:** No need to manually remove or add signatures. The workflow handles the `sign` tool and keys automatically.

## 🛠️ How to Build Custom Firmware

You don't need any programming knowledge or local tools. Just follow these steps:

1. Go to the **Actions** tab of this repository.
2. Click on **"Huawei Balong ModFW Kitchen (Universal Builder)"** in the left sidebar.
3. Click the **"Run workflow"** button.
4. Fill in the dropdowns:
   * **Original Firmware:** Select the base firmware (stored in GitHub Releases).
   * **Balong Family:** Select the correct family (e.g., `v7r11`, `v7r2`, `v7r1`, etc.).
   * **Device Overlay:** Select your specific device (e.g., `e8372h-153_zong_21.333.64.00.1456`).
   * **Firmware Version:** Enter the version (e.g., `21.333.64.00.1456`).
   * **WEBUI Version:** Enter the WEBUI version (e.g., `WEBUI_17.100.20.01.1456_RE5`).
5. Click **"Run workflow"**.
6. Wait for the process to finish (usually 1-2 minutes).
7. Go to the **Releases** section on the right side of the repository. 
8. Download your custom firmware! (It will be named like `E8372h_..._modfw.bin`).

## ✨ Features Included in the Custom Firmware

The modifications applied automatically include:
* ADB and Telnet access
* IPv6 support on mobile networks
* Full-fledged `busybox`, `iptables`, and `ip6tables`
* `atc` utility (for sending AT commands)
* TTL/HL fix scripts (`ttl`) and IMEI change scripts (`imei`)
* DNS over TLS (stubby) and DNS-level adblock
* Unlocked NVRAM items
* OpenVPN and `curl`
* Entware repository installation script
* And much more!

## ⚠️ Disclaimer

Flashing custom firmware carries risks. Ensure you have a backup and understand the flashing process for your specific device. Use this kitchen at your own risk.
