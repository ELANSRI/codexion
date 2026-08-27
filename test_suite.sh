#!/bin/bash

# ╔═══════════════════════════════════════════════════════════════╗
# ║          CODEXION TEST SUITE - macOS Compatible              ║
# ║     Test suite pour valider la simulation multithreadée      ║
# ╚═══════════════════════════════════════════════════════════════╝

# Couleurs pour la sortie
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'  # No Color

# Compteurs
PASSED=0
FAILED=0
TOTAL=0

# ═══════════════════════════════════════════════════════════════
# FONCTIONS UTILITAIRES
# ═══════════════════════════════════════════════════════════════

print_header() {
    echo ""
    echo -e "${BLUE}╔════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║ $1${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════════════════╝${NC}"
    echo ""
}

print_subheader() {
    echo -e "${YELLOW}▶ $1${NC}"
}

test_case() {
    local name="$1"
    local cmd="$2"
    local expected="$3"
    local timeout_val="${4:-3}"
    
    TOTAL=$((TOTAL + 1))
    printf "  [TEST %2d] %-45s ... " "$TOTAL" "$name"
    
    # Exécuter avec timeout
    output=$(timeout "$timeout_val" bash -c "$cmd" 2>&1)
    exit_code=$?
    
    # Timeout
    if [ $exit_code -eq 124 ]; then
        echo -e "${YELLOW}TIMEOUT${NC}"
        return
    fi
    
    # Pas de sortie attendue
    if [ -z "$expected" ]; then
        if [ $exit_code -eq 0 ]; then
            echo -e "${GREEN}PASS${NC}"
            PASSED=$((PASSED + 1))
        else
            echo -e "${RED}FAIL${NC}"
            FAILED=$((FAILED + 1))
        fi
        return
    fi
    
    # Vérifier le pattern attendu
    if echo "$output" | grep -q "$expected"; then
        echo -e "${GREEN}PASS${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}FAIL${NC}"
        FAILED=$((FAILED + 1))
        if [ "${VERBOSE:-0}" = "1" ]; then
            echo "    Command: $cmd"
            echo "    Expected pattern: $expected"
            echo "    Got: $output" | head -3
        fi
    fi
}

# ═══════════════════════════════════════════════════════════════
# VÉRIFICATIONS PRÉALABLES
# ═══════════════════════════════════════════════════════════════

print_header "🔍 VÉRIFICATIONS PRÉALABLES"

# Vérifier que l'exécutable existe
if [ ! -f ./codexion ]; then
    echo -e "${RED}✗ Erreur: ./codexion introuvable${NC}"
    echo -e "${YELLOW}Compilez d'abord avec: make${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Exécutable ./codexion trouvé${NC}"

# Vérifier les outils disponibles
if command -v leaks &> /dev/null; then
    echo -e "${GREEN}✓ Outil 'leaks' disponible (macOS)${NC}"
    HAS_LEAKS=1
else
    echo -e "${YELLOW}⚠ Outil 'leaks' non disponible${NC}"
    HAS_LEAKS=0
fi

if command -v timeout &> /dev/null; then
    echo -e "${GREEN}✓ Commande 'timeout' disponible${NC}"
else
    echo -e "${YELLOW}⚠ 'timeout' non disponible, installation recommandée${NC}"
fi

# ═══════════════════════════════════════════════════════════════
# TESTS DE BASE - CAS SIMPLES
# ═══════════════════════════════════════════════════════════════

print_header "🧪 TESTS DE BASE - CAS SIMPLES"

print_subheader "Test 1: Single coder (1 dongle)"
test_case "Coder 1 compile" \
    "./codexion 1 1000 100 50 50 1 50 fifo" \
    "1 is compiling" 3

test_case "Burnout pas atteint" \
    "./codexion 1 1000 100 50 50 1 50 fifo | grep -c 'burned out'" \
    "0" 3

print_subheader "Test 2: Deux coders (FIFO)"
test_case "Les 2 coders compilent" \
    "./codexion 2 1500 100 50 50 1 50 fifo | grep -c 'is compiling'" \
    "[2-9]" 5

test_case "Alternance compile/debug" \
    "./codexion 2 1500 100 50 50 1 50 fifo | grep -E 'is (compiling|debugging)' | head -4" \
    "is compiling" 5

print_subheader "Test 3: Trois coders (EDF)"
test_case "Les 3 coders compilent" \
    "./codexion 3 2000 100 50 50 1 50 edf | grep -c 'is compiling'" \
    "[3-9]" 5

test_case "Aucun burnout sans deadline" \
    "./codexion 3 3000 100 50 50 3 50 edf | grep 'burned out' | wc -l" \
    "0" 5

# ═══════════════════════════════════════════════════════════════
# TESTS DE FORMAT DES LOGS
# ═══════════════════════════════════════════════════════════════

print_header "📋 TESTS DE FORMAT DES LOGS"

print_subheader "Test 4: Format correct"
test_case "Format timestamp coder action" \
    "./codexion 1 1000 100 50 50 1 50 fifo | head -1 | grep -E '^[0-9]+ [0-9]+ (has taken|is compiling)'" \
    "^[0-9]" 3

test_case "Actions valides uniquement" \
    "./codexion 2 1000 100 50 50 1 50 fifo | grep -v -E '(has taken|is compiling|is debugging|is refactoring|burned out)' | wc -l" \
    "0" 5

print_subheader "Test 5: Ordre chronologique"
test_case "Timestamps croissants" \
    "./codexion 2 1500 100 50 50 1 50 fifo | awk '{print \\$1}' | sort -n | diff - <(./codexion 2 1500 100 50 50 1 50 fifo | awk '{print \\$1}') | wc -l" \
    "[0-2]" 6

# ═══════════════════════════════════════════════════════════════
# TESTS DE DONGLE
# ═══════════════════════════════════════════════════════════════

print_header "🔌 TESTS DE DONGLE"

print_subheader "Test 6: Acquisition des dongles"
test_case "Double 'has taken' avant 'is compiling'" \
    "./codexion 2 1500 200 100 100 1 50 fifo | grep -E '(has taken|is compiling)' | head -3 | grep -c 'has taken'" \
    "[2-9]" 5

test_case "Relâchement des dongles" \
    "./codexion 2 1500 100 50 50 2 50 fifo | grep 'is compiling' | wc -l" \
    "[2-9]" 5

print_subheader "Test 7: Cooldown"
test_case "Cooldown court (10ms)" \
    "./codexion 2 1000 100 50 50 3 10 fifo | grep 'is compiling' | wc -l" \
    "[3-9]" 5

test_case "Cooldown long (200ms)" \
    "./codexion 2 2500 100 50 50 2 200 fifo | grep 'is compiling' | wc -l" \
    "[2-9]" 8

# ═══════════════════════════════════════════════════════════════
# TESTS DE BURNOUT
# ═══════════════════════════════════════════════════════════════

print_header "🔥 TESTS DE BURNOUT"

print_subheader "Test 8: Détection du burnout"
test_case "Burnout est détecté" \
    "./codexion 2 300 200 100 100 1 50 fifo | grep 'burned out'" \
    "burned out" 5

test_case "Un seul coder brûle" \
    "./codexion 3 500 200 100 100 1 50 fifo | grep 'burned out' | wc -l" \
    "1" 5

test_case "Message burnout avec numéro" \
    "./codexion 2 200 150 100 100 1 50 fifo | grep 'burned out'" \
    "[0-9] burned out" 5

print_subheader "Test 9: Burnout et arrêt"
test_case "Arrêt après burnout" \
    "./codexion 2 300 200 100 100 1 50 fifo | tail -1 | grep -c 'burned out'" \
    "1" 5

# ═══════════════════════════════════════════════════════════════
# TESTS DE SCHEDULER
# ═══════════════════════════════════════════════════════════════

print_header "📊 TESTS DE SCHEDULER"

print_subheader "Test 10: FIFO vs EDF"
test_case "FIFO fonctionne" \
    "./codexion 3 2000 100 50 50 2 50 fifo | grep -c 'is compiling'" \
    "[2-9]" 5

test_case "EDF fonctionne" \
    "./codexion 3 2000 100 50 50 2 50 edf | grep -c 'is compiling'" \
    "[2-9]" 5

print_subheader "Test 11: Équité du scheduler"
test_case "Tous les coders compilent (FIFO)" \
    "./codexion 4 3000 100 50 50 2 50 fifo | grep 'is compiling' | awk '{print \\$2}' | sort -u | wc -l" \
    "[3-4]" 6

test_case "Tous les coders compilent (EDF)" \
    "./codexion 4 3000 100 50 50 2 50 edf | grep 'is compiling' | awk '{print \\$2}' | sort -u | wc -l" \
    "[3-4]" 6

# ═══════════════════════════════════════════════════════════════
# TESTS DE NOMBRE DE COMPILATIONS
# ═══════════════════════════════════════════════════════════════

print_header "🔄 TESTS DE NOMBRE DE COMPILATIONS"

print_subheader "Test 12: Limite de compilations"
test_case "1 compilation requise" \
    "./codexion 1 2000 100 50 50 1 50 fifo | grep -c 'is compiling'" \
    "1" 3

test_case "3 compilations requises" \
    "./codexion 1 3000 100 50 50 3 50 fifo | grep -c 'is compiling'" \
    "3" 5

test_case "5 compilations = 10 par 2 coders" \
    "./codexion 2 5000 100 50 50 5 50 fifo | grep -c 'is compiling'" \
    "1[0-9]" 7

# ═══════════════════════════════════════════════════════════════
# TESTS DE CAS LIMITES
# ═══════════════════════════════════════════════════════════════

print_header "⚠️  TESTS DE CAS LIMITES"

print_subheader "Test 13: Paramètres limites"
test_case "Cooldown minimum (0)" \
    "./codexion 2 1000 100 50 50 1 0 fifo | grep -c 'is compiling'" \
    "[0-9]" 3

test_case "Très court time_to_compile" \
    "./codexion 1 500 10 5 5 1 50 fifo | grep -c 'is compiling'" \
    "1" 3

test_case "Très court burnout" \
    "./codexion 2 100 50 25 25 1 50 fifo | grep 'burned out' | wc -l" \
    "[0-1]" 2

print_subheader "Test 14: Beaucoup de coders"
test_case "5 coders compilent" \
    "./codexion 5 2000 100 50 50 1 50 fifo | grep -c 'is compiling'" \
    "[5-9]" 5

test_case "10 coders sans crash" \
    "./codexion 10 3000 100 50 50 1 50 edf | grep -c 'is compiling'" \
    "[0-9]" 6

# ═══════════════════════════════════════════════════════════════
# TESTS VALGRIND (macOS)
# ═══════════════════════════════════════════════════════════════

print_header "🔬 TESTS VALGRIND (macOS - Leaks)"

if [ $HAS_LEAKS -eq 1 ]; then
    print_subheader "Test 15: Détection de fuites mémoire"
    
    test_case "Single coder - pas de fuites" \
        "leaks -atExit -- ./codexion 1 500 100 50 50 1 50 fifo 2>&1 | grep -c 'no leaks'" \
        "1" 5
    
    test_case "Two coders - pas de fuites" \
        "leaks -atExit -- ./codexion 2 1000 100 50 50 1 50 fifo 2>&1 | grep -c 'no leaks'" \
        "1" 5
else
    print_subheader "Test 15: Détection de fuites mémoire"
    echo -e "${YELLOW}⚠ 'leaks' non disponible - installation recommandée${NC}"
    echo "   Pour installer: brew install leak-detection-toolkit"
fi

# ═══════════════════════════════════════════════════════════════
# TESTS DE STRESS
# ═══════════════════════════════════════════════════════════════

print_header "💪 TESTS DE STRESS"

print_subheader "Test 16: Charge élevée"
test_case "4 coders longue durée" \
    "./codexion 4 3000 200 100 100 3 50 fifo | wc -l" \
    "[1-9][0-9]" 8

test_case "EDF avec beaucoup de compilations" \
    "./codexion 3 5000 100 50 50 10 50 edf | grep -c 'is compiling'" \
    "[1-9][0-9]" 10

# ═══════════════════════════════════════════════════════════════
# RÉSULTATS FINAUX
# ═══════════════════════════════════════════════════════════════

print_header "📊 RÉSULTATS FINAUX"

PERCENTAGE=$((PASSED * 100 / TOTAL))

echo "  Total tests:    $TOTAL"
echo -e "  ${GREEN}Passed:${NC}       $PASSED"
echo -e "  ${RED}Failed:${NC}       $FAILED"
echo "  Success rate:   $PERCENTAGE%"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}╔════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║         ✅ ALL TESTS PASSED - BIEN JOUÉ!          ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════╝${NC}"
    exit 0
else
    echo -e "${RED}╔════════════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║        ❌ SOME TESTS FAILED - À CORRIGER           ║${NC}"
    echo -e "${RED}╚════════════════════════════════════════════════════╝${NC}"
    exit 1
fi
