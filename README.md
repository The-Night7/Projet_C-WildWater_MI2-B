# 🌊 C-WildWater: Hydraulic Network Analysis

> **Massive Data Processing & C Algorithms**

![Language](https://img.shields.io/badge/Language-C-blue) ![Script](https://img.shields.io/badge/Script-Bash-green) ![Build](https://img.shields.io/badge/Build-Make-orange)

## 📖 Project Overview

**C-WildWater** is a high-performance application designed to analyze a drinking water distribution network that simulates **1/3 of the French network**.

The project processes a massive data file (several million lines, >500MB), combining **Shell** flexibility and **C** performance to:
1. Ingest and structure data (Graphs & AVL Trees)
2. Generate statistics on treatment plants
3. Detect leaks and calculate network losses
4. Visualize results through dynamic graphs

## 🚀 Key Features

### 📊 1. Volume Analysis (Histo Mode)
Generates CSV files and graphs via **Gnuplot** to visualize:
* **Capacity:** Maximum volume plants can process
* **Source:** Water volume drawn from sources
* **Actual:** Final distributed volume (after leaks)
* **All:** Combined histogram showing all three states simultaneously

### 💧 2. Leak Calculation (Leaks Mode)
Optimized graph traversal algorithm (DFS) to calculate total water volume lost downstream from a specific plant:
* Optimized processing time (milliseconds)
* Accounts for leak percentages at each section
* Automatically identifies critical section (worst leak in absolute value)

## 🛠️ Setup & Requirements

This project is designed for **Linux** environments (or WSL).

### Clone the project
```bash
git clone https://github.com/The-Night7/Projet_C-WildWater_MI2-B.git
cd Projet_C-WildWater_MI2-B
```

**Dependencies:**
```bash
sudo apt update
sudo apt install build-essential gnuplot make
sudo apt install dos2unix
```

## Usage:
```bash
dos2unix scripts/myScript.sh
chmod +x scripts/myScript.sh
./scripts/myScript.sh histo max    # Maximum capacity
./scripts/myScript.sh histo src    # Source volume
./scripts/myScript.sh histo real   # Actual processed volume
./scripts/myScript.sh histo all    # Combined graph of all three modes
```

**Leaks example:**
```bash
./scripts/myScript.sh leaks "Facility complex #RH400057F"
```