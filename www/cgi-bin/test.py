#!/usr/bin/env python3

import os
import sys

method = os.environ.get("REQUEST_METHOD", "GET")
query = os.environ.get("QUERY_STRING", "")

UPLOAD_DIR = "./www/upload/"

print("Content-Type: text/html\n")

print("<html><body>")
print("<h1>CGI Python Works 🎉</h1>")
print(f"<p>Method: {method}</p>")

# ================= GET =================
if method == "GET":
    print(f"<p>Query: {query}</p>")

# ================= POST =================
elif method == "POST":
    length = int(os.environ.get("CONTENT_LENGTH", 0))
    body = sys.stdin.read(length) if length > 0 else ""

    # filename via query (?file=test.txt) ou fallback
    filename = "default.txt"
    if "file=" in query:
        filename = query.split("file=")[1].split("&")[0]

    path = os.path.join(UPLOAD_DIR, filename)

    try:
        with open(path, "w") as f:
            f.write(body)
        print(f"<p>Saved to: {filename}</p>")
        print(f"<p>Content: {body}</p>")
    except Exception as e:
        print(f"<p>Error writing file: {e}</p>")

# ================= DELETE =================
elif method == "DELETE":
    filename = ""
    if "file=" in query:
        filename = query.split("file=")[1].split("&")[0]

    path = os.path.join(UPLOAD_DIR, filename)

    if filename == "":
        print("<p>No file specified</p>")
    elif os.path.exists(path):
        try:
            os.remove(path)
            print(f"<p>Deleted: {filename}</p>")
        except Exception as e:
            print(f"<p>Error deleting file: {e}</p>")
    else:
        print("<p>File not found</p>")

# ================= UNKNOWN =================
else:
    print("<p>Unsupported method</p>")

print("</body></html>")