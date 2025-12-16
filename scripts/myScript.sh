#!/bin/bash

# -----------------------------------------------------------------------------
# Analysis script for C-WildWater project
#
# Features:
# - Compiles the project if necessary
# - Generates histograms (mode 'histo')
# - Calculates downstream leakage from facilities (mode 'leaks')
# - Exports CSV and creates PNG graphs using gnuplot
#
# Usage: ./scripts/myScript.sh [<data_file>] <mode> <argument>
#   - <data_file>: Optional, defaults to data/c-wildwater_v3.dat
#   - <mode>: 'histo' (histogram) or 'leaks' (leakage)
#   - <argument>:
#     * In 'histo' mode: max, src, real or all
#     * In 'leaks' mode: facility ID or comma-separated facility list
# -----------------------------------------------------------------------------

# Navigate to project root
cd "$(dirname "$0")/.." || exit 1

# Path configuration
DATA_DIR="data"
SRC_DIR="src"
BIN_DIR="src/bin"
GRAPH_DIR="$DATA_DIR/output_images"
EXEC_MAIN="$BIN_DIR/c-wildwater"
LOG_FILE="$DATA_DIR/processing.log"
DEFAULT_INPUT="$DATA_DIR/c-wildwater_v3.dat"
CACHE_DIR="$DATA_DIR/.cache"

# Create necessary directories
mkdir -p "$GRAPH_DIR" "$DATA_DIR" "$BIN_DIR" "$CACHE_DIR"

# Display colors
BLUE='\033[0;34m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
BOLD='\033[1m'
RESET='\033[0m'

# Display usage information
usage() {
    cat <<EOF
    ${BOLD}Usage:${RESET} $0 [<data_file>] <histo|leaks> <parameter>

      ${BOLD}<data_file>${RESET}      Path to .dat or .csv file (optional).
                          If omitted, uses $DEFAULT_INPUT.
      ${BOLD}histo${RESET}            Generates histogram (parameter: max|src|real|all).
      ${BOLD}leaks${RESET}            Calculates leakage for specified facility.
                          Parameter: facility name or comma-separated list.

    ${BOLD}Examples:${RESET}
      $0 histo max
      $0 histo all
      $0 data/my_file.dat histo src
      $0 leaks Facility\ complex\ #RH400057F
      $0 data/my_file.dat leaks "Facility complex #RH400057F"
      $0 leaks "Facility A,Facility B,Facility C"
EOF
    exit 1
}

# Display progress bar
show_progress() {
    local current=$1
    local total=$2
    local width=50
    local percent=$((current * 100 / total))
    local completed=$((width * current / total))

    printf "\r[%-${width}s] %3d%%" "$(printf "%0.s#" $(seq 1 $completed))" "$percent"
}

# Log and display progress messages
log_progress() {
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo -e "${BLUE}[${timestamp}]${RESET} $1" | tee -a "$LOG_FILE"
}

# Display error messages
log_error() {
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo -e "${RED}[${timestamp}] ERROR:${RESET} $1" | tee -a "$LOG_FILE"
}

# Display success messages
log_success() {
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo -e "${GREEN}[${timestamp}] SUCCESS:${RESET} $1" | tee -a "$LOG_FILE"
}

# Argument validation
if [ "$#" -lt 2 ]; then
    usage
fi

# Initialize log
echo "=== Processing started $(date '+%Y-%m-%d %H:%M:%S') ===" > "$LOG_FILE"

# Start global timer (in ms)
GLOBAL_START=$(date +%s%3N)

# Parse arguments: data file and mode
if [ "$1" = "histo" ] || [ "$1" = "leaks" ]; then
    DATAFILE="$DEFAULT_INPUT"
    COMMAND="$1"
    PARAM="$2"
    shift 2
else
    DATAFILE="$1"
    COMMAND="$2"
    PARAM="$3"
    shift 3
fi

log_progress "Mode: ${BOLD}$COMMAND${RESET}, Parameter: ${BOLD}$PARAM${RESET}, File: ${BOLD}$DATAFILE${RESET}"

# Check if data file exists
if [ ! -f "$DATAFILE" ]; then
    log_error "File '$DATAFILE' does not exist."
    exit 1
fi

# Check if trying to use "leaks all" mode (not supported)
if [ "$COMMAND" = "leaks" ] && [ "$PARAM" = "all" ]; then
    log_error "Mode 'leaks all' is not supported. Please specify a facility name instead."
    exit 1
fi

# Always run make clean before compilation
log_progress "Running make clean..."
echo -ne "${YELLOW}"
(cd "$SRC_DIR" && make clean) || {
    echo -ne "${RESET}"
    log_error "Make clean failed."
    exit 1
}
echo -ne "${RESET}"
log_success "Make clean completed successfully"

# Compile program
log_progress "Compiling program..."
echo -ne "${YELLOW}"
(cd "$SRC_DIR" && make) || {
    echo -ne "${RESET}"
    log_error "Compilation failed."
    exit 1
}
echo -ne "${RESET}"
log_success "Compilation completed successfully"

chmod +x "$EXEC_MAIN" 2>/dev/null

# Display visual separator
show_separator() {
    echo -e "\n${YELLOW}----------------------------------------${RESET}\n"
}

# Process according to selected mode
case "$COMMAND" in
    histo)
        # Validate histogram parameter
        case "$PARAM" in
            max|src|real|all) ;;
            *)
                log_error "Histogram mode must be 'max', 'src', 'real' or 'all'."
                exit 1
                ;;
        esac

        show_separator
        log_progress "Histogram Mode (${BOLD}$PARAM${RESET})"

        # Generate CSV file
        OUT_CSV="$DATA_DIR/vol_${PARAM}.csv"
        echo -e "${YELLOW}Generating data...${RESET}"

        # Execute with simulated progress bar
        "$EXEC_MAIN" "$DATAFILE" "$PARAM" > "$DATA_DIR/.temp_output" &
        PID=$!

        # Display progress indicator during processing
        i=0
        spin='-\|/'
        while kill -0 $PID 2>/dev/null; do
            i=$(( (i+1) % 4 ))
            printf "\r[%c] Processing data..." "${spin:$i:1}"
            sleep 0.1
        done

        wait $PID
        RESULT_CODE=$?

        if [ $RESULT_CODE -ne 0 ]; then
            printf "\r%s\n" "$(printf ' %.0s' {1..50})"  # Clear line
            log_error "Data generation failed."
            exit 1
        fi

        # Sort data
        printf "\r[#] Sorting data...                 "
        LC_ALL=C sort -t';' -k2,2g "$DATA_DIR/.temp_output" > "$OUT_CSV"
        rm -f "$DATA_DIR/.temp_output"
        printf "\r%s\n" "$(printf ' %.0s' {1..50})"  # Clear line

        if [ ! -s "$OUT_CSV" ]; then
            log_error "Empty CSV (No data generated)."
            exit 1
        fi

        log_success "Data generated and sorted successfully"

# --- BRANCH: "ALL" BONUS MODE or STANDARD MODES ---
        if [ "$PARAM" = "all" ]; then
            # ---------------------------------------------------------
            # BONUS 1: Combined Histogram (Capacity / Source / Real)
            # ---------------------------------------------------------
            
            # Temporary files
            TMP_MAX="$DATA_DIR/.tmp_max"
            TMP_SRC="$DATA_DIR/.tmp_src"
            TMP_REAL="$DATA_DIR/.tmp_real"
            TMP_MERGED="$DATA_DIR/.tmp_merged"
            TMP_SORTED="$DATA_DIR/.tmp_sorted"

            echo -e "${YELLOW}Generating combined data...${RESET}"

            # 1. Generate the 3 files separately
            "$EXEC_MAIN" "$DATAFILE" "max"  > "$TMP_MAX"
            "$EXEC_MAIN" "$DATAFILE" "src"  > "$TMP_SRC"
            "$EXEC_MAIN" "$DATAFILE" "real" > "$TMP_REAL"

            # 2. Merge into: ID;Max;Source;Real
            # Use awk to join files on ID. Missing values default to 0.
            awk -F';' '
                FNR==1 { file_idx++ }
                file_idx==1 { max[$1] = $2; next }
                file_idx==2 { src[$1] = $2; next }
                file_idx==3 { real[$1] = $2; next }
                END {
                    OFS=";"
                    for (id in max) {
                        m = max[id] + 0
                        s = (id in src) ? src[id] + 0 : 0
                        r = (id in real) ? real[id] + 0 : 0
                        if (m > 0) print id, m, s, r
                    }
                }
            ' "$TMP_MAX" "$TMP_SRC" "$TMP_REAL" > "$TMP_MERGED"

            # 3. ASCENDING SORT (Small -> Big) on capacity (column 2)
            # This ensures consistency with standard modes (Left to Right display)
            sort -t';' -k2,2g "$TMP_MERGED" > "$TMP_SORTED"

            # Cleanup intermediate files
            rm -f "$TMP_MAX" "$TMP_SRC" "$TMP_REAL" "$TMP_MERGED"

            if [ ! -s "$TMP_SORTED" ]; then
                log_error "Error generating combined data."
                exit 1
            fi

            # =========================================================
            # GRAPH 1: TOP 10 (BIGGEST)
            # =========================================================
            # We take the END of the sorted file (tail)
            IMG_BIG="$GRAPH_DIR/vol_all_big.png"
            GP_DATA_BIG="$DATA_DIR/data_all_big.dat"
            
            tail -n 10 "$TMP_SORTED" > "$GP_DATA_BIG"

            echo -e "${YELLOW}Generating Top 10 graph (Stacked)...${RESET}"

            gnuplot -persist <<EOF
set terminal png size 1200,800
set output '$IMG_BIG'
set title "Top 10 Stations - Bonus Mode (Smallest to Largest)"
set style data histograms
set style histogram rowstacked
set style fill solid 1.0 border -1
set datafile separator ';'
set ylabel 'Volume (M.m3)'
set xtics rotate by -45 font ',10'
set key outside top center horizontal
set grid y
set boxwidth 0.7 relative

# Stacking: Blue (Bottom) -> Red (Middle) -> Green (Top)
# Using ternary operators to handle potential negative values if data is inconsistent
plot '$GP_DATA_BIG' using 4:xtic(1) title 'Real Output' lc rgb '#3366CC', \
     '' using (\$3-\$4 < 0 ? 0 : \$3-\$4) title 'Losses' lc rgb '#DC3912', \
     '' using (\$2-\$3 < 0 ? 0 : \$2-\$3) title 'Unused Capacity' lc rgb '#109618'
EOF
            if [ -f "$IMG_BIG" ]; then
                log_success "Top 10 Graph generated: ${BOLD}$IMG_BIG${RESET}"
            fi

            # =========================================================
            # GRAPH 2: BOTTOM 50 (SMALLEST)
            # =========================================================
            # We take the START of the sorted file (head)
            IMG_SMALL="$GRAPH_DIR/vol_all_small.png"
            GP_DATA_SMALL="$DATA_DIR/data_all_small.dat"
            
            head -n 50 "$TMP_SORTED" > "$GP_DATA_SMALL"

            echo -e "${YELLOW}Generating Bottom 50 graph (Stacked)...${RESET}"

            gnuplot -persist <<EOF
set terminal png size 1600,900
set output '$IMG_SMALL'
set title "Bottom 50 Stations - Bonus Mode (Smallest to Largest)"
set style data histograms
set style histogram rowstacked
set style fill solid 1.0 border -1
set datafile separator ';'
set ylabel 'Volume (M.m3)'
set xtics rotate by -90 font ',8'
set key outside top center horizontal
set grid y
set boxwidth 0.8 relative

# Same stacking logic
plot '$GP_DATA_SMALL' using 4:xtic(1) title 'Real Output' lc rgb '#3366CC', \
     '' using (\$3-\$4 < 0 ? 0 : \$3-\$4) title 'Losses' lc rgb '#DC3912', \
     '' using (\$2-\$3 < 0 ? 0 : \$2-\$3) title 'Unused Capacity' lc rgb '#109618'
EOF
            if [ -f "$IMG_SMALL" ]; then
                log_success "Bottom 50 Graph generated: ${BOLD}$IMG_SMALL${RESET}"
            fi

            # Final cleanup
            rm -f "$TMP_SORTED" "$GP_DATA_BIG" "$GP_DATA_SMALL"

        else
            # ---------------------------------------------------------
            # STANDARD MODES (max, src, real)
            # ---------------------------------------------------------
            echo -e "${YELLOW}Creating histograms...${RESET}"
            
            # Ensure the file is sorted in ASCENDING order (Small -> Big)
            # -k2,2g = numeric sort on column 2
            SORTED_STD="$DATA_DIR/.sorted_std_asc"
            sort -t';' -k2,2g "$OUT_CSV" > "$SORTED_STD"

            # 1. Top 10 (Largest)
            # Since file is sorted Small -> Big, Top 10 are at the END (tail)
            GP_BIG="$DATA_DIR/data_big.dat"
            tail -n 10 "$SORTED_STD" > "$GP_BIG"
            IMG_BIG="$GRAPH_DIR/vol_${PARAM}_big.png"

            gnuplot -persist <<EOF
set terminal png size 1200,800
set output '$IMG_BIG'
set title "Top 10 Stations ($PARAM) - Smallest to Largest"
set yrange [0:*]
set style data histograms
set style fill solid 1.0 border -1
set datafile separator ';'
set ylabel 'Volume (M.m3)'
set xtics rotate by -45
set grid y
# Gnuplot preserves file order (Left=Top of file, Right=Bottom of file)
plot '$GP_BIG' using 2:xtic(1) title 'Volume' lc rgb 'blue'
EOF
            log_success "Top 10 image generated: ${BOLD}$IMG_BIG${RESET}"

            # 2. Bottom 50 (Smallest)
            # Since file is sorted Small -> Big, Bottom 50 are at the START (head)
            GP_SMALL="$DATA_DIR/data_small.dat"
            head -n 50 "$SORTED_STD" > "$GP_SMALL"
            IMG_SMALL="$GRAPH_DIR/vol_${PARAM}_small.png"

            gnuplot -persist <<EOF
set terminal png size 1600,900
set output '$IMG_SMALL'
set title "Bottom 50 Stations ($PARAM) - Smallest to Largest"
set key outside top center horizontal
set style data histograms
set boxwidth 0.8 relative
set style fill solid 1.0 border -1
set datafile separator ';'
set ylabel 'Volume (M.m3)'
set format y "%.4f"
set yrange [*:*]
set xtics rotate by -90 font ',8'
set grid y
plot '$GP_SMALL' using 2:xtic(1) title 'Volume' lc rgb 'red'
EOF
            log_success "Bottom 50 image generated: ${BOLD}$IMG_SMALL${RESET}"

            # Clean up temporary files
            rm -f "$GP_BIG" "$GP_SMALL" "$SORTED_STD"
        fi
        ;;

    leaks)
        show_separator
        log_progress "Leaks Mode (${BOLD}$PARAM${RESET})"
        LEAK_FILE="$DATA_DIR/leaks.dat"
        CACHE_FILE="$CACHE_DIR/.leaks_cache.dat"
        touch "$CACHE_FILE"

        # Internal function to process a facility - Optimized version
        process_factory() {
            local FACTORY="$1"
            # Clean up spaces
            FACTORY=$(echo "$FACTORY" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')

            echo -e "\n${YELLOW}Processing facility:${RESET} ${BOLD}$FACTORY${RESET}"

            # Generate unique filename based on hash of facility name
            # to avoid conflicts with special characters
            local FACTORY_HASH=$(echo "$FACTORY" | md5sum | cut -d' ' -f1)
            local TEMP_ERR_FILE="$CACHE_DIR/err_${FACTORY_HASH}.tmp"
            local TEMP_OUT_FILE="$CACHE_DIR/out_${FACTORY_HASH}.tmp"

            # Check Cache - Optimized to search for exact facility name
            CACHED_VAL=$(grep -F "$(echo "$FACTORY" | sed 's/;/\\;/g');" "$CACHE_FILE" | tail -n1 | cut -d';' -f2)
            if [ -n "$CACHED_VAL" ]; then
                echo -e "${GREEN}[CACHE]${RESET} Result found in cache"
                if ! grep -q "$(echo "$FACTORY" | sed 's/;/\\;/g');" "$LEAK_FILE" 2>/dev/null; then
                    # Add only result in facility;value format to leaks.dat
                    echo "$FACTORY;$CACHED_VAL" >> "$LEAK_FILE"
                fi
                echo -e "${BOLD}Leak volume for $FACTORY:${RESET} ${BLUE}${CACHED_VAL} M.m3${RESET}"
                return
            fi

            # C calculation with progress bar - Optimized version
            echo -e "${YELLOW}Calculation in progress...${RESET}"
            T_START=$(date +%s%3N)

            # Use nice to reduce process priority and ulimit to limit memory usage
            # Also use stdbuf to disable output buffering
            nice -n 10 stdbuf -oL -eL "$EXEC_MAIN" "$DATAFILE" "$FACTORY" > "$TEMP_OUT_FILE" 2> "$TEMP_ERR_FILE" &
            PID=$!

            # Display animated progress bar with timeout
            i=0
            spin='-\|/'
            TIMEOUT=180  # 3 minutes max
            START_TIME=$SECONDS

            while kill -0 $PID 2>/dev/null; do
                # Check if timeout is reached
                ELAPSED=$((SECONDS - START_TIME))
                if [ $ELAPSED -gt $TIMEOUT ]; then
                    echo -e "\n${RED}Timeout reached (${TIMEOUT}s). Forcing calculation to stop.${RESET}"
                    kill -9 $PID 2>/dev/null
                    break
                fi

                i=$(( (i+1) % 4 ))

                # Get progress information if available
                if [ -f "$TEMP_ERR_FILE" ]; then
                    PROGRESS_INFO=$(grep -o "Lines processed: [0-9]*" "$TEMP_ERR_FILE" | tail -n1)
                    if [ -n "$PROGRESS_INFO" ]; then
                        printf "\r[%c] %s (${ELAPSED}s)" "${spin:$i:1}" "$PROGRESS_INFO"
                    else
                        printf "\r[%c] Processing... (${ELAPSED}s)" "${spin:$i:1}"
                    fi
                else
                    printf "\r[%c] Processing... (${ELAPSED}s)" "${spin:$i:1}"
                fi

                sleep 0.2
            done

            # Wait for process to finish and get value
            wait $PID
            EXIT_CODE=$?

            # Get value from output file
            if [ -f "$TEMP_OUT_FILE" ]; then
                VAL=$(cat "$TEMP_OUT_FILE" | tr -d '\n\r')
            else
                VAL=""
            fi

            # If file is empty or process was interrupted
            if [ -z "$VAL" ] || [ $EXIT_CODE -ne 0 ]; then
                # Try with simpler and more direct approach
                echo -e "\n${YELLOW}Retrying with alternative method...${RESET}"
                VAL=$("$EXEC_MAIN" "$DATAFILE" "$FACTORY" 2>/dev/null | tr -d '\n\r')
            fi

            T_END=$(date +%s%3N)
            DURATION=$((T_END - T_START))

            # Clear progress line
            printf "\r%s\n" "$(printf ' %.0s' {1..70})"

            if [ "$VAL" = "-1" ] || [ -z "$VAL" ]; then
                log_error "Facility '$FACTORY' not found or calculation failed (${DURATION}ms)"
                # Add only result in facility;value format to leaks.dat and cache
                echo "$FACTORY;-1" >> "$LEAK_FILE"
                echo "$FACTORY;-1" >> "$CACHE_FILE"
            else
                log_success "Calculation completed in ${DURATION}ms"
                # Add only result in facility;value format to leaks.dat and cache
                echo "$FACTORY;$VAL" >> "$LEAK_FILE"
                echo "$FACTORY;$VAL" >> "$CACHE_FILE"
                echo -e "${BOLD}Leak volume for $FACTORY:${RESET} ${BLUE}${VAL} M.m3${RESET}"
                if [ -f "$TEMP_ERR_FILE" ]; then
                    if grep -q "BONUS INFO" "$TEMP_ERR_FILE"; then
                         echo -e "\n${YELLOW}=== DETECTION TRONCON CRITIQUE ===${RESET}"
                         # On extrait les lignes entre les bornes du bonus
                         sed -n '/=== BONUS INFO ===/,/=================/p' "$TEMP_ERR_FILE" | sed '1d;$d'
                         echo -e "${YELLOW}==================================${RESET}"
                    fi
                fi
            fi

            # Cleanup
            rm -f "$TEMP_ERR_FILE" "$TEMP_OUT_FILE"
        }

        # Process multiple facilities (comma-separated)
        if [[ "$PARAM" == *","* ]]; then
            IFS=',' read -ra FACTORIES <<< "$PARAM"
            TOTAL_FACTORIES=${#FACTORIES[@]}

            echo -e "${YELLOW}Processing $TOTAL_FACTORIES facilities...${RESET}"

            for ((i=0; i<TOTAL_FACTORIES; i++)); do
                FAC="${FACTORIES[$i]}"
                echo -e "\n${YELLOW}[$((i+1))/$TOTAL_FACTORIES]${RESET}"
                process_factory "$FAC"
            done

            # Display summary
            echo -e "\n${BOLD}Leak volume summary:${RESET}"
            TOTAL_LEAKS=0
            for FAC in "${FACTORIES[@]}"; do
                FAC=$(echo "$FAC" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
                VAL=$(grep -F "$(echo "$FAC" | sed 's/;/\\;/g');" "$LEAK_FILE" | tail -n1 | cut -d';' -f2)
                if [ -n "$VAL" ] && [ "$VAL" != "-1" ]; then
                    echo -e "- ${BOLD}$FAC:${RESET} ${BLUE}${VAL} M.m3${RESET}"
                    # Use bc for decimal calculations
                    if command -v bc &>/dev/null; then
                        TOTAL_LEAKS=$(echo "$TOTAL_LEAKS + $VAL" | bc)
                    else
                        # Fallback if bc isn't available
                        TOTAL_LEAKS=$(awk "BEGIN {print $TOTAL_LEAKS + $VAL}")
                    fi
                fi
            done
            echo -e "\n${BOLD}Total leaks:${RESET} ${BLUE}${TOTAL_LEAKS} M.m3${RESET}"

        # Process single facility
        else
            process_factory "$PARAM"
        fi

        # Optimize leak file (remove duplicates)
        if [ -f "$LEAK_FILE" ]; then
            # Use sort -u to remove duplicates in one pass
            sort -u -t';' -k1,1 "$LEAK_FILE" > "${LEAK_FILE}.tmp"
            mv "${LEAK_FILE}.tmp" "$LEAK_FILE"

            # Verify file only contains lines in facility;value format
            if grep -v "^[^;]*;[^;]*$" "$LEAK_FILE" > /dev/null; then
                log_error "leaks.dat contains lines with incorrect format."
                # Filter to keep only valid lines
                grep "^[^;]*;[^;]*$" "$LEAK_FILE" > "${LEAK_FILE}.clean"
                mv "${LEAK_FILE}.clean" "$LEAK_FILE"
            fi
        fi
        ;;

    *)
        log_error "Unknown command ('$COMMAND'). Use 'histo' or 'leaks'."
        exit 1
        ;;
esac

# End global timer
GLOBAL_END=$(date +%s%3N)
TOTAL_DURATION=$((GLOBAL_END - GLOBAL_START))

show_separator
log_success "Processing completed in ${BOLD}${TOTAL_DURATION} ms${RESET}"
echo "=== Processing ended $(date '+%Y-%m-%d %H:%M:%S') ===" >> "$LOG_FILE"

exit 0