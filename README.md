# Gambit Chess Engine Handbook

Gambit is a GUI-only chess application written in C++ and organized
around an MVC architecture.\
This repository is intended to be distributed as a ready-to-run Windows
package.

------------------------------------------------------------------------

## Overview

Gambit provides a classic desktop chess experience with board view, move
history, captured pieces, timers, and promotion handling.

------------------------------------------------------------------------

## Architecture

-   Model → Chess state and rules\
-   View → SDL rendering\
-   Controller → Game flow

------------------------------------------------------------------------

## How to Run

1.  Extract ZIP\
2.  Open MVC/\
3.  Run gambit.exe

------------------------------------------------------------------------

## Controls

-   N → New game\
-   M → Toggle mode\
-   H → Help\
-   Esc → Quit

------------------------------------------------------------------------

# MVC Migration Report

## Executive Summary

-   Strict MVC structure\
-   Controller handles logic\
-   View handles UI\
-   Model handles state

------------------------------------------------------------------------

## MVC Diagram

``` mermaid
graph LR
    CMA[CoreModelAdapter]
    EngineCore[Engine Core]
    CI[ControllerImpl]
    SV[SDLView]

    SV -->|InputEvent| CI
    CI -->|calls| CMA
    CI -->|updates UI| SV
    CMA -->|wraps| EngineCore
```
