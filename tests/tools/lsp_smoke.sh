set -eu

LSP=${LSP:-build/ari-lsp}
ARI=${ARI:-build/ari}
FILE=$(pwd)/examples/count.ari
URI="file://${FILE}"
VALID_TEXT='pub struct Point { \n  mut x: i64,\n}\n\nfn main() -> i64 {\n  return 0;\n}\n'
INVALID_TEXT='fn main() -> i64 {\n  let text = format!(\"value={}\", 1);\n  return 0;\n}\n'

send() {
  body=$1
  length=$(printf '%s' "$body" | wc -c)
  printf 'Content-Length: %s\r\n\r\n%s' "$length" "$body"
}

output=$(
  {
    send '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{}}'
    send '{"jsonrpc":"2.0","method":"textDocument/didOpen","params":{"textDocument":{"uri":"'"$URI"'","languageId":"ari","version":1,"text":"'"$VALID_TEXT"'"}}}'
    send '{"jsonrpc":"2.0","id":2,"method":"textDocument/documentSymbol","params":{"textDocument":{"uri":"'"$URI"'"}}}'
    send '{"jsonrpc":"2.0","id":3,"method":"textDocument/hover","params":{"textDocument":{"uri":"'"$URI"'"},"position":{"line":4,"character":4}}}'
    send '{"jsonrpc":"2.0","id":4,"method":"textDocument/definition","params":{"textDocument":{"uri":"'"$URI"'"},"position":{"line":4,"character":4}}}'
    send '{"jsonrpc":"2.0","id":5,"method":"textDocument/completion","params":{"textDocument":{"uri":"'"$URI"'"},"position":{"line":6,"character":0}}}'
    send '{"jsonrpc":"2.0","method":"textDocument/didChange","params":{"textDocument":{"uri":"'"$URI"'","version":2},"contentChanges":[{"text":"'"$INVALID_TEXT"'"}]}}'
    send '{"jsonrpc":"2.0","id":6,"method":"textDocument/diagnostic","params":{"textDocument":{"uri":"'"$URI"'"}}}'
    send '{"jsonrpc":"2.0","id":7,"method":"shutdown","params":null}'
    send '{"jsonrpc":"2.0","method":"exit","params":null}'
  } | "$LSP" --ari "$ARI" --rule trailing-whitespace=error
)

printf '%s' "$output" | grep -q '"method":"textDocument/publishDiagnostics"'
printf '%s' "$output" | grep -q '"kind":"full"'
printf '%s' "$output" | grep -q '"documentSymbolProvider":true'
printf '%s' "$output" | grep -q '"hoverProvider":true'
printf '%s' "$output" | grep -q '"definitionProvider":true'
printf '%s' "$output" | grep -q '"completionProvider"'
printf '%s' "$output" | grep -q '"source":"ari-lint","code":"lint/trailing-whitespace"'
printf '%s' "$output" | grep -q '"severity":1,"source":"ari-lint","code":"lint/trailing-whitespace"'
printf '%s' "$output" | grep -q '"name":"Point"'
printf '%s' "$output" | grep -q '"name":"main"'
printf '%s' "$output" | grep -q '"label":"Point"'
printf '%s' "$output" | grep -q '"label":"main"'
printf '%s' "$output" | grep -q 'Ari function `main`'
printf '%s' "$output" | grep -q '"id":4,"result":{"uri":"'"$URI"'"'
printf '%s' "$output" | grep -q '"line":4'
printf '%s' "$output" | grep -q "prelude macro 'format!' needs owned runtime strings"
printf '%s' "$output" | grep -q '"line":1'
