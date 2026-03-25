AGENTS.md

Este documento lista comandos recomendados para build, teste e lint do projeto
openkylinmenueditor (C++/Qt - qmake). Use localmente ou em CI.

Build
- Instalar dependências do sistema: Qt (qt5/qt6), build-essential, pkg-config.
- Build com qmake + make (raiz do repositório):
  ```
  qmake openkylinmenueditor.pro
  make -j$(nproc)
  ```
- Build por subdiretório:
  ```
  (cd libukmge && qmake && make -j$(nproc))
  (cd shell && qmake && make -j$(nproc))
  ```
- Limpeza:
  ```
  make clean
  (cd shell && make clean)
  (cd libukmge && make clean)
  ```

Testes
- Estado atual: não há testes automatizados detectados.
- Recomendações:
  - Adicionar testes unitários com GoogleTest ou Catch2 e integrar com CTest.
  - Com CTest configurado execute:
    ```
    ctest --output-on-failure
    ```
  - Ou criar um alvo `make test` que execute binários de verificação.

Lint / Análise Estática / Formatação
- cppcheck (análise estática):
  ```
  cppcheck --enable=all --inconclusive shell libukmge
  ```
- clang-tidy (requer compile_commands.json):
  ```
  clang-tidy -p=build shell/*.cpp libukmge/**/*.cpp
  ```
- clang-format (verificação/format):
  ```
  find shell libukmge -name '*.cpp' -o -name '*.h' | xargs clang-format -style=file --dry-run --Werror
  ```
  aplicar formato:
  ```
  find shell libukmge -name '*.cpp' -o -name '*.h' | xargs clang-format -style=file -i
  ```

Checks úteis adicionais
- validar arquivos TS/XML (i18n):
  ```
  xmllint --noout shell/res/i18n/*.ts
  ```
- validar .desktop:
  ```
  desktop-file-validate shell/openkylinmenueditor.desktop
  ```
- checar arquivos PO:
  ```
  msgfmt -c -o /dev/null po/*.po
  ```

Sugestão de CI (GitHub Actions) - alto nível
- Steps:
  1) setup (Ubuntu) + instalar Qt dev packages e build-essential
  2) qmake + make -j
  3) rodar `cppcheck` e `clang-format --dry-run` (falhar se formato incorreto)
  4) rodar `ctest` se testes adicionados

Notas
- Projeto atual é C++/Qt; não é necessário Python para build principal.
- Se quiser, eu posso:
  1) ajustar este arquivo para Qt6 ou outro fluxo de build, ou
  2) gerar um GitHub Actions workflow que execute os passos acima.
