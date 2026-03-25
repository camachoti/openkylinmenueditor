---
applyTo: '**'
---

# Planejamento: Melhoria Visual do OpenKylin Menu Editor (Referência Peony)

**Data:** (continue amanhã)

## Pendências para próximas ações

1. **Janela Arredondada**
   - Analisar como é implementada a janela arredondada do Peony (HeaderBar e main window)
   - Replicar fielmente a solução para o OpenKylin Menu Editor, garantindo que tanto a máscara física quanto o visual tenham o mesmo comportamento e suavidade.

2. **Transparência Real**
   - Investigar como Peony aplica a transparência real da barra/window (QWidget ou QWindow?), especialmente o efeito de "acompanhamento" da cor/base do que está atrás.
   - Aplicar a mesma técnica, para que a barra do nosso projeto reaja igual ao contexto visual do desktop.

3. **Visual do Search File (Peony)**
   - Verificar o visual detalhado do campo/botão Search File no Peony.
   - Ajustar o Search do tree do nosso editor para ficar idêntico, incluindo ícone, borda, cantos, animações e feedback.

4. **Dimensão Padrão da Janela**
   - Ajustar o tamanho padrão de abertura do programa para ser um pouco maior na horizontal, comparando proporções e valores usados pelo Peony.

---

**Prioridade:**
- Máxima fidelidade visual e comportamental, usando Peony como referência canônica.
- Documentar diferenças e etapas de adaptação para futuros contribuidores.

*(Atualize este arquivo conforme avançar!)*
