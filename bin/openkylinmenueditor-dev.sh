#!/bin/bash
# Script para rodar openkylinmenueditor em modo desenvolvimento
# Garante que a biblioteca libukmge.so.1 seja encontrada sem instalar no sistema

# Descobre o diretório do script, mesmo com symlink
THISDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export LD_LIBRARY_PATH="$THISDIR/../libukmge${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
exec "$THISDIR/openkylinmenueditor" "$@"
