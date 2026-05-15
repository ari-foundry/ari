set -eu

LSP=${LSP:-build/ari-lsp}
ARI=${ARI:-build/ari}
FILE=$(pwd)/tests/errors/prelude-macro-format-planned.ari
URI="file://${FILE}"

send() {
  body=$1
  length=$(printf '%s' "$body" | wc -c)
  printf 'Content-Length: %s\r\n\r\n%s' "$length" "$body"
}

output=$(
  {
    send '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{}}'
    send '{"jsonrpc":"2.0","method":"textDocument/didOpen","params":{"textDocument":{"uri":"'"$URI"'","languageId":"ari","version":1,"text":""}}}'
    send '{"jsonrpc":"2.0","id":2,"method":"textDocument/diagnostic","params":{"textDocument":{"uri":"'"$URI"'"}}}'
    send '{"jsonrpc":"2.0","id":3,"method":"shutdown","params":null}'
    send '{"jsonrpc":"2.0","method":"exit","params":null}'
  } | "$LSP" --ari "$ARI"
)

printf '%s' "$output" | grep -q '"method":"textDocument/publishDiagnostics"'
printf '%s' "$output" | grep -q '"kind":"full"'
printf '%s' "$output" | grep -q "prelude macro 'format!' needs owned runtime strings"
printf '%s' "$output" | grep -q '"line":1'
