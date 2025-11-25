#!/bin/bash

# ==========================================
# Script para configurar vcan0 en Linux
# ==========================================

# 1. Verificar si el usuario es root (necesario para ip link y modprobe)
if [ "$EUID" -ne 0 ]; then
  echo "❌ Error: Por favor, ejecuta este script como root (sudo)."
  exit 1
fi

echo "--- Iniciando configuración de vcan0 ---"

# 2. Cargar el módulo del kernel 'vcan' si no está cargado
if ! lsmod | grep -q "^vcan"; then
    echo "-> Cargando módulo vcan..."
    modprobe vcan
    if [ $? -ne 0 ]; then
        echo "❌ Error al cargar el módulo vcan."
        exit 1
    fi
else
    echo "-> El módulo vcan ya estaba cargado."
fi

# 3. Verificar si vcan0 ya existe
if ip link show vcan0 > /dev/null 2>&1; then
    echo "-> vcan0 ya existe. Eliminando para reiniciar..."
    ip link delete vcan0
fi

# 4. Crear la interfaz
echo "-> Creando interfaz vcan0..."
ip link add dev vcan0 type vcan

if [ $? -ne 0 ]; then
    echo "❌ Error al crear la interfaz."
    exit 1
fi

# 5. Levantar la interfaz (UP)
echo "-> Levantando la interfaz..."
ip link set up vcan0

# 6. Verificación final
if ip link show vcan0 | grep -q "UP"; then
    echo "✅ ¡Éxito! El puerto vcan0 está creado y activo."
    echo "--------------------------------------------"
    ip -details link show vcan0
    echo "--------------------------------------------"
else
    echo "❌ Error: La interfaz se creó pero no está activa."
    exit 1
fi