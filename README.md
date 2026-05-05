# 0714-02-CSE-2100  
**Course Code:** 0714 02 CSE 2100  
**Course Title:** Advanced Programming Laboratory  
**ID:** 240204, 240215  

---

# MVC Migration Report

## Executive Summary
- The codebase is now structured using a strict **Model-View-Controller (MVC)** architecture.  
- All GUI-side game-flow logic has been moved into the controller.  
- The UI is responsible only for rendering and input translation.  
- The model owns all game state and rules.  

---

## Objectives
- Enforce a toolkit-agnostic model interface  
- Centralize input handling and game flow in the controller  
- Keep rendering and platform dependencies inside the view layer  
- Preserve protocol support (UCI, XBoard) on the same model API  

---

## Target Scope
- **Project root:** `SOLID`  
- **Modes covered:** GUI, UCI, XBoard  

---

## Architecture Overview

### Model (`IModel`)
- Owns chess state and exposes core operations  
- Implemented by `CoreModelAdapter`  

### View (`IView`)
- Handles rendering and input  
- Implemented by `SDLView`  
- Does NOT modify model state  

### Controller (`IController`)
- Handles game flow and logic  
- Implemented by `ControllerImpl`  

---

## MVC Interaction Diagram

```mermaid
graph LR
    CMA[CoreModelAdapter]
    EngineCore[Engine Core]
    CI[ControllerImpl]
    SV[SDLView]

    SV -->|InputEvent| CI
    CI -->|calls| CMA
    CI -->|updates UI| SV
    CMA -->|wraps| EngineCore
