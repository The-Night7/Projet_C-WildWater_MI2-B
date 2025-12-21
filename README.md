# 🌊 C-WildWater: Hydraulic Network Analysis

> **Pre-Ing 2 Project (2025-2026)**
> *Big Data Processing, AVL Structures, C Optimization & Gnuplot Visualization.*

---

## 📖 About the Project

Welcome to **C-WildWater**. This project was born from a technical challenge: to design a software pipeline capable of analyzing a massive CSV file describing the entire water distribution network in France.

The goal was not just to read data, but to understand it: detecting stations operating beyond capacity, isolating sources, and above all, spotting potential leaks in the network. To achieve this, we combined the power of **C** (for performance and memory management), the flexibility of **Shell** (for automation), and the precision of **Gnuplot** (for visualization).

## 🚀 Features

The program processes data to produce analyses in the form of charts and reports:

*   **📊 Histo Mode (Statistics):**
    *   **Station:** Ranking stations by capacity and flow.
    *   **Consumption:** Station analysis (Capacity vs. Actual Load).
    *   **Sources:** Identification and analysis of network origin points.
*   **💧 Leaks Mode:**
    *   Detection of sections where outgoing water quantity is less than incoming quantity.
    *   Identification of "Worst Sections".
*   **🌟 Bonus Mode (Advanced Analysis):**
    *   Stacked Charts visualization showing the exact breakdown: *Real Volume* vs. *Losses* vs. *Unused Capacity*.

## 🛠️ Installation and Usage

### Prerequisites

To compile and run the project, you need **GCC**, **Make**, and **Gnuplot**. Here is how to install them depending on your system:

**🐧 Linux (Ubuntu / Debian / WSL)**
```bash
sudo apt update
sudo apt install build-essential gnuplot
```

**🍎 macOS (via Homebrew)**
```bash
brew install gcc make gnuplot
```

**🎩 Fedora / RHEL**
```bash
sudo dnf install gcc make gnuplot
```

### Quick Start

Everything is controlled by our master script `myScript.sh`. It handles compilation, cleanup, and execution.

1.  **Grant execution rights:**
    ```bash
    chmod +x myScript.sh
    ```

2.  **Launch analysis (Help):**
    ```bash
    ./myScript.sh help
    ```

3.  **Command Examples:**
    *   *Generate histograms:*
        ```bash
        ./myScript.sh histo all/src/real/max
        ```
    *   *Detect leaks:*
        ```bash
        ./myScript.sh leaks "Factory complex #ID"
        ```

The generated charts (`.png` files) will be saved in the dedicated folder (`data/output_images/`).

## ⚙️ Technical Choices

To ensure execution speed on millions of lines:

*   **AVL Trees:** We use balanced binary search trees to store and retrieve stations instantly (O(log n) complexity), avoiding the slowness of a classic linked list.
*   **Optimized Sequential Processing:** Carefully designed algorithms with early termination conditions and memory-efficient data structures to handle large datasets efficiently.
*   **Robust Parsing:** Native handling of CSV irregularities (spaces, variable formats).

## 👥 The Team

This project is the result of close collaboration where each member brought their expertise:

*   **Myriam Bensaid** 🏗️
    *   *Architecture & Optimization:* She structured the C code, developed critical utility functions, and managed the Git flow (merges) between branches. She also led the optimization of data processing and terminal display.

*   **Sheryne Ouarghi** 📈
    *   *Visualization & Data:* She mastered Gnuplot to transform our raw data into readable charts. She ensured visual results were relevant and professionally formatted.

*   **Matthieu Vannereau** 🧠
    *   *Business Logic & Algorithms:* He designed the algorithms for leak detection and "worst section" calculation. He also prepared the data logic necessary for the bonus (stacked histograms) mode.

## 👏 Credits

*   **Optimization Techniques:** Thanks to various online resources and academic papers that inspired our implementation of efficient sequential processing algorithms.

---
*C-WildWater Project - 2025*