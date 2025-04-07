#!/bin/zsh

SOURCE="bully_monitor.c"
OUTPUT="bully"

# Compilar se necessário
if [ ! -f $OUTPUT ]; then
    echo "🛠️ Compilando $SOURCE..."
    gcc $SOURCE -o $OUTPUT -lpthread
    if [ $? -ne 0 ]; then
        echo "❌ Erro ao compilar."
        exit 1
    fi
fi

# Caminho do diretório atual
CUR_DIR=$(pwd)

osascript <<EOF
tell application "iTerm2"
    set newWindow to (create window with default profile)
    tell current session of newWindow
        write text "cd $CUR_DIR; ./$OUTPUT 1"
        
        set sess2 to split vertically with default profile
        tell sess2 to write text "cd $CUR_DIR; ./$OUTPUT 2"
        
        set sess3 to split vertically with default profile
        tell sess3 to write text "cd $CUR_DIR; ./$OUTPUT 3"
        
        set sess4 to split vertically with default profile
        tell sess4 to write text "cd $CUR_DIR; ./$OUTPUT 4"
        
        set sess5 to split vertically with default profile
        tell sess5 to write text "cd $CUR_DIR; ./$OUTPUT 5"
    end tell
end tell
EOF
