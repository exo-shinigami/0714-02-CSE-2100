# Gambit Chess - New Features Quick Guide

## 🎮 How to Use the New Features

### 1. Move Highlighting (Green Highlights)

**What it does:** Shows all legal moves for a selected piece

**How to use:**
1. Click on any piece (it must be your turn and your color)
2. The selected square will turn **YELLOW**
3. All possible destination squares will be highlighted in **GREEN**
4. Click on any green square to move there
5. Click the piece again to deselect

**Example:**
```
Click on a knight → See all 8 possible L-shaped moves highlighted in green
Click on a rook   → See all horizontal and vertical moves highlighted
Click on a pawn   → See forward moves and diagonal captures
```

---

### 2. King in Check (Red Highlight)

**What it does:** Automatically highlights your king when in danger

**How it works:**
- If your king is in check, the king's square turns **BRIGHT RED**
- This happens automatically - no action needed
- The red highlight disappears once you move out of check
- Works for both White and Black

**Visual Indicator:**
```
Normal King Square: Regular board color
King in Check:      🔴 Bright Red Background
```

---

### 3. Pawn Promotion (Choose Your Piece!)

**What it does:** Lets you choose which piece to promote your pawn to

**How to use:**
1. Move a pawn to the opposite end of the board (8th rank for White, 1st rank for Black)
2. A **promotion dialog** will appear showing 4 choices:
   - ♕/♛ **Queen** (most powerful)
   - ♖/♜ **Rook** (strong for endgame)
   - ♗/♝ **Bishop** (useful for diagonal control)
   - ♘/♞ **Knight** (great for tactics)
3. Click on the piece you want OR press the keyboard shortcut:
   - **Q** = Queen
   - **R** = Rook
   - **B** = Bishop
   - **N** = Knight
4. Your pawn transforms into the chosen piece

**Pro Tip:** Queen is the strongest choice in most situations, but sometimes a knight is better to avoid stalemate or deliver checkmate!

---

### 4. Captured Pieces Display (Side-by-Side)

**What it shows:** All pieces captured during the game

**New Layout:**
```
┌─────────────────────┐
│    Captured         │
├──────────┬──────────┤
│  Black   │  White   │
│    ♟     │    ♙     │
│    ♞     │    ♗     │
│    ♜     │    ♘     │
│          │    ♖     │
└──────────┴──────────┘
```

**Features:**
- Black pieces on the LEFT
- White pieces on the RIGHT
- Pieces stack vertically within each column
- Automatically scrolls if too many captures
- No overlap with other UI elements

---

## 🎯 Visual Reference

### Color Coding System

| Element | Color | Meaning |
|---------|-------|---------|
| Yellow Square | 🟨 | Your selected piece |
| Green Squares | 🟩 | Legal moves available |
| Red Square | 🟥 | Your king is in check! |
| Gray Squares | ⬜⬛ | Normal board squares |

---

## 💡 Tips & Tricks

1. **Planning Moves:**
   - Click pieces to see where they can go before committing
   - Use the green highlights to plan multi-move strategies

2. **Avoiding Blunders:**
   - The red king highlight warns you of danger
   - Make sure to resolve check immediately

3. **Pawn Promotion Strategy:**
   - Usually promote to Queen (most powerful)
   - Promote to Knight if it gives immediate checkmate
   - Promote to Rook/Bishop in special endgame situations
   - Under-promotion can prevent stalemate

4. **Captured Pieces:**
   - Track material advantage by comparing the columns
   - More captured opponent pieces = better position

---

## 🐛 Troubleshooting

**Q: The green highlights don't show up**
- A: Make sure you clicked on YOUR piece (correct color for your turn)

**Q: I can't select any pieces**
- A: Check if the game is over or if it's not your turn

**Q: The promotion dialog won't go away**
- A: You must choose a piece to continue the game

**Q: My king looks red but I'm not sure why**
- A: Your king is in check! You must move it to safety or block the attack

---

## 🎲 Example Gameplay

1. **Opening:** Click pawn → See green squares 1 or 2 spaces ahead → Move
2. **Mid-game:** Click knight → See L-shaped green moves → Capture opponent piece
3. **Check:** Opponent attacks your king → Red highlight appears → Move king to safety
4. **Promotion:** Your pawn reaches the end → Dialog appears → Click Queen → Continue playing
5. **Checkmate:** Your king is red and no moves available → Game over!

---

## ⌨️ Keyboard Shortcuts

| Key | Action |
|-----|--------|
| **N** | New Game |
| **M** | Switch Mode (PvP/PvE) |
| **H** | Show Help |
| **Q** | Promote to Queen (during promotion) |
| **R** | Promote to Rook (during promotion) |
| **B** | Promote to Bishop (during promotion) |
| **N** | Promote to Knight (during promotion) |

---

Enjoy the enhanced chess experience! 🎉
