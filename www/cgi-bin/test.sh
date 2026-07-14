#!/bin/bash

UPLOAD_DIR="./www/upload"

echo "Content-Type: text/html"
echo ""

echo "<html><body>"
echo "<h1>CGI Bash Works 🎉</h1>"
echo "<p>Method: $REQUEST_METHOD</p>"

# helper pour récupérer file=xxx
get_file_from_query() {
    printf "%s" "$QUERY_STRING" | cut -d'=' -f2 | cut -d'&' -f1
}

# ================= GET =================
if [ "$REQUEST_METHOD" = "GET" ]; then
    echo "<p>Query: $QUERY_STRING</p>"
fi

# ================= POST =================
if [ "$REQUEST_METHOD" = "POST" ]; then
    read -n "$CONTENT_LENGTH" BODY

    FILE=$(get_file_from_query)
    [ -z "$FILE" ] && FILE="default.txt"

    PATH_FILE="$UPLOAD_DIR/$FILE"

    if echo "$BODY" > "$PATH_FILE"; then
        echo "<p>Saved to: $FILE</p>"
        echo "<p>Content: $BODY</p>"
    else
        echo "<p>Error writing file</p>"
    fi
fi

# ================= DELETE =================
if [ "$REQUEST_METHOD" = "DELETE" ]; then
    FILE=$(get_file_from_query)
    PATH_FILE="$UPLOAD_DIR/$FILE"

    echo "<p>Trying to delete: $PATH_FILE</p>"

    if [ -z "$FILE" ]; then
        echo "<p>No file specified</p>"

    elif [ -f "$PATH_FILE" ]; then
        if rm "$PATH_FILE"; then
            echo "<p>Deleted: $FILE</p>"
        else
            echo "<p>Delete failed</p>"
        fi

    else
        echo "<p>File not found</p>"
    fi
fi

echo "</body></html>"