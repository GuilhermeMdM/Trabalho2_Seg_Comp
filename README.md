# Trabalho2 Seg Comp - RSA com OAEP

Implementação em C++ de:

- Geração de chaves RSA (`p`, `q`, `n`, `e`, `d`)
- Cifração/decifração RSA usando OAEP (SHA-256 + MGF1)
- Leitura da mensagem a partir de arquivo externo

## Requisitos

- `g++` com suporte a C++17
- Ambiente com suporte a threads POSIX (`-pthread`)

## Compilação

No diretório do projeto, execute:

```bash
g++ -std=c++17 -O2 RSA.cpp keygen.cpp oaep.cpp -pthread -o rsa
```

## Uso

A execução recebe o caminho de um arquivo de mensagem:

```bash
./rsa mensagem_teste.txt
```

## Formato da mensagem

- A mensagem e lida como bytes (nao apenas texto ASCII).
- Arquivo vazio retorna erro.

## Limite de tamanho (OAEP)

O limite máximo da mensagem e:

```text
mLen <= k - 2*hLen - 2
```

Onde:

- `k` = tamanho do modulo RSA em bytes
- `hLen` = tamanho do hash em bytes (SHA-256 => `32`)

Exemplo para RSA 2048 bits (`k = 256`):

- `mLen <= 256 - 64 - 2 = 190 bytes`

Se o arquivo exceder o limite, o programa retorna erro de tamanho da mensagem para OAEP.

## Saida esperada

Durante a execução, o programa:

- Gera `p` e `q` em paralelo
- Mostra os valores de `p`, `q`, `e`, `d`
- Exibe o texto cifrado
- Exibe a mensagem recuperada apos decifração e OAEP decode

## Observação

Esta implementação e para estudo/experimento acadêmico.
