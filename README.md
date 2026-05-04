# Trabalho2 Seg Comp - RSA com OAEP e Assinatura SHA-3

Implementação em C++ de:

- Geração de dois pares de chaves RSA (`p`, `q`, `n`, `e`, `d`): emissor e receptor
- Cifração/decifração RSA usando OAEP (SHA-256 + MGF1)
- Assinatura e verificação de assinatura usando hash SHA3-256 + RSA
- Leitura da mensagem a partir de arquivo externo

## Requisitos

- `g++` com suporte a C++17
- Ambiente com suporte a threads POSIX (`-pthread`)

## Compilação

No diretório do projeto, execute:

```bash
g++ -std=c++17 -O2 main.cpp RSA.cpp signature.cpp keygen.cpp oaep.cpp sha3.cpp base64.cpp -pthread -o rsa
```

## Uso

A execução recebe o caminho de um arquivo de mensagem:

```bash
./rsa mensagem_teste.txt
```

Durante a execução, o programa:

- Cifra/decifra a mensagem com RSA+OAEP usando o par de chaves do receptor (Parte I)
- Calcula hash SHA3-256 da mensagem e assina com a chave privada do emissor (Parte II)
- Lê `assinatura.sig`, verifica com a chave pública do emissor e compara hashes (Parte III)

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

- Gera `p` e `q` em paralelo
- Mostra os valores de `p`, `q`, `e`, `d`
- Exibe o texto cifrado OAEP (Base64)
- Exibe a mensagem recuperada apos OAEP decode
- Salva assinatura em `assinatura.sig`
- Exibe resultado da verificacao da assinatura

## Observação

Esta implementação e para estudo/experimento acadêmico.
