#pragma once
#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"HTMLPAGE(
<!DOCTYPE html>
<html lang="th" class="light">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
    <title>SmartVest · เสื้อกั๊กระบายความร้อนอัจฉริยะ (3D Fan & Controller)</title>
    <link rel="manifest" href="manifest.webmanifest">
    <meta name="theme-color" content="#0284c7">

    <!-- Pinned Library Versions (No @latest) -->
    <script src="https://cdn.tailwindcss.com/3.4.16"></script>
    <script src="https://cdn.jsdelivr.net/npm/lucide@0.469.0/dist/umd/lucide.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/three.js/r128/three.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.7/dist/chart.umd.min.js"></script>
    <link href="https://fonts.googleapis.com/css2?family=Prompt:wght@300;400;500;600;700;800&display=swap" rel="stylesheet">

    <script>
        tailwind.config = {
            darkMode: 'class',
            theme: {
                extend: {
                    fontFamily: {
                        sans: ['Prompt', '-apple-system', 'BlinkMacSystemFont', 'sans-serif'],
                    },
                    colors: {
                        brand: {
                            50: '#f0f9ff',
                            100: '#e0f2fe',
                            200: '#bae6fd',
                            300: '#7dd3fc',
                            400: '#38bdf8',
                            500: '#0ea5e9',
                            600: '#0284c7',
                            700: '#0369a1',
                            800: '#075985',
                            900: '#0c4a6e',
                        }
                    }
                }
            }
        }
    </script>

    <style>
        body {
            font-family: 'Prompt', -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            transition: background-color 0.25s ease, color 0.25s ease;
            user-select: none;
            -webkit-user-select: none;
            touch-action: manipulation;
        }

        /* Light Theme */
        html.light body {
            background-color: #f8fafc;
            color: #0f172a;
        }
        html.light .glass-card {
            background: #ffffff;
            border: 1.5px solid #e2e8f0;
            box-shadow: 0 4px 18px -2px rgba(148, 163, 184, 0.1);
        }

        /* Dark Theme */
        html.dark body {
            background-color: #0b111a;
            color: #f1f5f9;
        }
        html.dark .glass-card {
            background: #141c28;
            border: 1.5px solid #223043;
            box-shadow: 0 4px 20px -2px rgba(0, 0, 0, 0.4);
        }

        .btn-touch-active:active { transform: scale(0.97); }

        /* Custom scrollbar */
        ::-webkit-scrollbar { width: 6px; height: 6px; }
        ::-webkit-scrollbar-track { background: transparent; }
        ::-webkit-scrollbar-thumb { background: #94a3b8; border-radius: 9999px; }

        /* Toast Banner */
        .toast-banner {
            position: fixed;
            bottom: 84px;
            left: 50%;
            transform: translateX(-50%) translateY(30px);
            background: #1e293b;
            color: #f8fafc;
            padding: 12px 22px;
            border-radius: 9999px;
            font-size: 14px;
            font-weight: 600;
            box-shadow: 0 10px 25px -3px rgba(0,0,0,0.3);
            z-index: 1000;
            opacity: 0;
            visibility: hidden;
            transition: all 0.25s cubic-bezier(0.16, 1, 0.3, 1);
            max-width: 90%;
            text-align: center;
            border: 1px solid #334155;
        }
        .toast-banner.show {
            opacity: 1;
            visibility: visible;
            transform: translateX(-50%) translateY(0);
        }
    </style>
</head>
<body class="min-h-screen flex flex-col pb-24">

    <!-- Top Navigation Bar -->
    <header class="sticky top-0 z-40 bg-white/90 dark:bg-slate-900/90 backdrop-blur-md border-b border-slate-200/80 dark:border-slate-800 px-3 sm:px-4 lg:px-8 py-2.5 transition-colors">
        <div class="max-w-7xl mx-auto flex flex-wrap items-center justify-between gap-2.5">
            
            <!-- Brand & Device Name -->
            <div class="flex items-center space-x-2.5">
                <div class="w-9 h-9 sm:w-10 sm:h-10 rounded-xl bg-gradient-to-tr from-brand-600 to-brand-400 flex items-center justify-center text-white shadow-md shadow-brand-500/20 shrink-0">
                    <i data-lucide="shield-check" class="w-5 h-5"></i>
                </div>
                <div>
                    <h1 class="text-sm sm:text-base font-bold text-slate-900 dark:text-white tracking-tight leading-tight flex items-center gap-1.5">
                        SmartVest <span class="text-[10px] sm:text-xs font-semibold px-2 py-0.5 rounded-full bg-brand-50 text-brand-600 dark:bg-brand-950/60 dark:text-brand-400 border border-brand-200 dark:border-brand-800">Air Flow</span>
                    </h1>
                    <div class="flex items-center space-x-1.5 text-[11px] text-slate-500 dark:text-slate-400 font-medium">
                        <span>ESP32-S3 BLE</span>
                        <span class="w-1 h-1 rounded-full bg-slate-300 dark:bg-slate-700"></span>
                        <span id="vest-id" class="text-brand-600 dark:text-brand-400 font-semibold">SmartVest-Test</span>
                    </div>
                </div>
            </div>

            <!-- Header Quick Actions -->
            <div class="flex items-center space-x-1.5 sm:space-x-2">
                <!-- Equipment BOM Button -->
                <button onclick="toggleEquipmentModal(true)" class="flex items-center space-x-1 px-2.5 py-1.5 rounded-xl bg-slate-100 hover:bg-slate-200 dark:bg-slate-800 dark:hover:bg-slate-700 text-slate-700 dark:text-slate-200 border border-slate-200 dark:border-slate-700 text-xs font-semibold transition-all" title="รายการอุปกรณ์ BOM">
                    <i data-lucide="cpu" class="w-4 h-4 text-brand-600 dark:text-brand-400"></i>
                    <span class="hidden sm:inline">อุปกรณ์</span>
                </button>

                <!-- Help '?' Button -->
                <button onclick="toggleHelpModal(true)" class="flex items-center justify-center w-8 h-8 rounded-xl bg-slate-100 hover:bg-slate-200 dark:bg-slate-800 dark:hover:bg-slate-700 text-slate-700 dark:text-slate-200 border border-slate-200 dark:border-slate-700 text-xs font-bold transition-all" title="คู่มือการใช้งาน">
                    ?
                </button>

                <!-- Developer Tools Button -->
                <button onclick="toggleDevModal(true)" class="flex items-center space-x-1 px-2.5 py-1.5 rounded-xl bg-slate-100 hover:bg-slate-200 dark:bg-slate-800 dark:hover:bg-slate-700 text-slate-700 dark:text-slate-200 border border-slate-200 dark:border-slate-700 text-xs font-semibold transition-all" title="เครื่องมือผู้พัฒนา">
                    <i data-lucide="wrench" class="w-4 h-4 text-amber-500"></i>
                    <span class="hidden sm:inline">ผู้พัฒนา</span>
                </button>

                <!-- BLE Status Chip / Connect Button -->
                <button id="btn-ble-connect" onclick="connectBLE()" class="flex items-center space-x-1.5 px-3 py-1.5 rounded-full bg-slate-100 dark:bg-slate-800 hover:bg-slate-200 dark:hover:bg-slate-700 border border-slate-300 dark:border-slate-700 text-slate-700 dark:text-slate-300 text-xs font-semibold transition-all">
                    <span id="ble-ping-dot" class="relative flex h-2 w-2">
                        <span class="relative inline-flex rounded-full h-2 w-2 bg-slate-400"></span>
                    </span>
                    <span id="ble-status-text">เชื่อมต่อ BLE</span>
                </button>

                <!-- Theme Toggle Button -->
                <button id="theme-toggle-btn" onclick="toggleTheme()" class="flex items-center justify-center p-2 rounded-xl bg-slate-100 hover:bg-slate-200 dark:bg-slate-800 dark:hover:bg-slate-700 text-slate-700 dark:text-amber-400 border border-slate-200 dark:border-slate-700 transition-all" title="สลับธีม สว่าง/มืด">
                    <i id="theme-icon" data-lucide="moon" class="w-4 h-4"></i>
                </button>
            </div>
        </div>
    </header>

    <!-- Disconnect Banner Warning -->
    <div id="disconn-alert" class="hidden bg-rose-50 dark:bg-rose-950/70 border-b border-rose-200 dark:border-rose-900 px-4 py-2 text-xs sm:text-sm font-semibold text-rose-800 dark:text-rose-200 text-center flex items-center justify-center gap-2">
        <i data-lucide="alert-triangle" class="w-4 h-4 text-rose-600 shrink-0"></i>
        <span>ขาดการเชื่อมต่อ Bluetooth! พัดลมอาจยังคงเป่าลมอยู่ที่ค่าล่าสุด แตะเชื่อมต่อใหม่เพื่อสั่งการ</span>
    </div>

    <!-- Main Content Flow -->
    <main class="flex-1 max-w-7xl w-full mx-auto p-3 sm:p-4 lg:p-6 space-y-4 sm:space-y-6">

        <!-- Top Layer: Hero Status Banner (Full-Width, High Visibility from Distance) -->
        <div id="hero-status-card" class="glass-card rounded-2xl p-4 sm:p-5 border-l-4 border-l-emerald-500 transition-colors">
            <div class="flex items-center justify-between mb-1.5">
                <div class="flex items-center gap-2.5">
                    <span id="hero-icon" class="text-2xl">🟢</span>
                    <div>
                        <h2 id="hero-title" class="text-lg sm:text-xl font-extrabold text-slate-900 dark:text-white leading-tight">พร้อมใช้งาน (สแตนด์บาย)</h2>
                        <span id="hero-tag" class="text-[11px] font-bold px-2 py-0.5 rounded-full bg-emerald-50 text-emerald-700 dark:bg-emerald-950/60 dark:text-emerald-400 border border-emerald-200 dark:border-emerald-800">ระบบปกติ</span>
                    </div>
                </div>
                <div id="hero-active-mode-badge" class="text-xs font-bold px-3 py-1 rounded-xl bg-slate-100 dark:bg-slate-800 text-slate-600 dark:text-slate-300">
                    โหมด: ควบคุมเอง
                </div>
            </div>
            <p id="hero-desc" class="text-xs sm:text-sm text-slate-600 dark:text-slate-300 leading-relaxed mt-1">
                พัดลมปิดอยู่ เลือกระดับความแรง 1–5 หรือเปิดระบบอัตโนมัติด้านล่างเพื่อเริ่มระบายความร้อน
            </p>

            <!-- Reset Button inside Hero (Visible only when stopped/tripped) -->
            <button id="btn-hero-reset" onclick="sendCmd(5, 0)" class="mt-3 w-full py-2.5 px-4 rounded-xl bg-rose-600 hover:bg-rose-700 text-white font-bold text-xs sm:text-sm shadow-md transition-all flex items-center justify-center gap-2 hidden">
                <i data-lucide="rotate-ccw" class="w-4 h-4"></i>
                <span>รีเซ็ตระบบ (RESET) เพื่อเริ่มทำงานใหม่</span>
            </button>
        </div>

        <!-- Row 2: 3D Fan Scene (~60%) + Combined Fan Control Box (~40%) -->
        <div class="grid grid-cols-1 lg:grid-cols-12 gap-4 sm:gap-6 items-start">

            <!-- 3D Fan Assemblies Scene (7 Cols ~60%) -->
            <div class="lg:col-span-7 glass-card rounded-2xl p-3 sm:p-4 lg:p-5 flex flex-col relative overflow-hidden transition-colors">
                <div class="flex flex-wrap items-center justify-between gap-2 mb-2 z-10">
                    <div>
                        <h2 class="text-xs sm:text-sm font-bold text-slate-800 dark:text-white flex items-center gap-1.5 sm:gap-2">
                            <i data-lucide="fan" class="w-4 h-4 text-brand-600 dark:text-brand-400"></i>
                            ชุดพัดลม 3D ตามคำสั่งจริง (Real Fans View)
                        </h2>
                        <p class="text-[11px] text-slate-500 dark:text-slate-400">พัดลมตัว 70mm (A/B), บ่า 40mm, แบต 30mm หมุนตามค่าจริง</p>
                    </div>

                    <!-- 3D Actions Overlay -->
                    <div class="flex items-center space-x-1 bg-slate-100 dark:bg-slate-800/90 p-1 rounded-xl border border-slate-200 dark:border-slate-700 text-xs">
                        <button onclick="resetCamera()" class="p-1.5 text-slate-600 dark:text-slate-300 hover:text-brand-600 hover:bg-white dark:hover:bg-slate-700 rounded-lg transition" title="รีเซ็ตมุมกล้อง">
                            <i data-lucide="rotate-ccw" class="w-3.5 h-3.5"></i>
                        </button>
                    </div>
                </div>

                <!-- 3D Canvas Container -->
                <div id="canvas-container" class="w-full h-[280px] sm:h-[340px] lg:h-[380px] rounded-xl bg-gradient-to-b from-slate-50 to-brand-50/20 dark:from-slate-900/80 dark:to-slate-800/50 relative flex items-center justify-center overflow-hidden border border-slate-100 dark:border-slate-800 cursor-grab active:cursor-grabbing touch-none">
                    <canvas id="three-canvas" class="w-full h-full"></canvas>

                    <!-- 2D Fallback Overlay (Shown if WebGL unsupported) -->
                    <div id="fallback-2d" class="hidden absolute inset-0 bg-slate-50 dark:bg-slate-900 flex flex-col items-center justify-center p-4 text-center z-20">
                        <i data-lucide="info" class="w-8 h-8 text-amber-500 mb-2"></i>
                        <p class="text-xs sm:text-sm font-bold text-slate-700 dark:text-slate-200">กำลังแสดงผลแบบ 2D (WebGL ไม่รองรับในเบราว์เซอร์นี้)</p>
                        <div class="flex items-center justify-center gap-6 mt-4">
                            <div class="flex flex-col items-center">
                                <div id="fan-2d-body" class="w-12 h-12 rounded-full border-2 border-brand-500 flex items-center justify-center"><i data-lucide="fan" class="w-7 h-7 text-brand-600"></i></div>
                                <span class="text-[10px] mt-1 font-semibold">พัดลมตัว</span>
                            </div>
                            <div class="flex flex-col items-center">
                                <div id="fan-2d-neck" class="w-10 h-10 rounded-full border-2 border-brand-400 flex items-center justify-center"><i data-lucide="fan" class="w-5 h-5 text-brand-500"></i></div>
                                <span class="text-[10px] mt-1 font-semibold">พัดลมบ่า</span>
                            </div>
                            <div class="flex flex-col items-center">
                                <div id="fan-2d-bat" class="w-8 h-8 rounded-full border-2 border-amber-500 flex items-center justify-center"><i data-lucide="fan" class="w-4 h-4 text-amber-500"></i></div>
                                <span class="text-[10px] mt-1 font-semibold">พัดลมแบต</span>
                            </div>
                        </div>
                    </div>

                    <!-- Gesture Hint -->
                    <div class="absolute bottom-2.5 left-2.5 bg-white/80 dark:bg-slate-900/80 backdrop-blur-sm border border-slate-200/80 dark:border-slate-700/80 rounded-lg px-2 py-1 text-[10px] text-slate-500 dark:text-slate-400 flex items-center gap-1 pointer-events-none">
                        <i data-lucide="mouse-pointer-2" class="w-3 h-3 text-brand-600 dark:text-brand-400"></i> ลากเพื่อหมุนมุมมองพัดลม
                    </div>

                    <!-- Live RPM Indicator Badge -->
                    <div class="absolute top-2.5 right-2.5 bg-white/85 dark:bg-slate-900/85 backdrop-blur-sm border border-slate-200/80 dark:border-slate-700/80 rounded-xl px-2.5 py-1 text-right pointer-events-none">
                        <div class="text-[9px] uppercase font-bold text-slate-400">พัดลมตัวจริง</div>
                        <div id="3d-fan-badge" class="text-xs font-black text-brand-600 dark:text-brand-400">0% (0 RPM)</div>
                    </div>
                </div>

                <!-- 3 Fan Labels Footer -->
                <div class="grid grid-cols-3 gap-2 mt-2 pt-2 border-t border-slate-100 dark:border-slate-800 text-center text-[11px] font-semibold text-slate-600 dark:text-slate-300">
                    <div>พัดลมตัว 70mm: <b id="lbl-rpm-body" class="text-brand-600 dark:text-brand-400">0%</b></div>
                    <div>พัดลมบ่า 40mm: <b id="lbl-rpm-neck" class="text-brand-600 dark:text-brand-400">0%</b></div>
                    <div>พัดลมแบต 30mm: <b id="lbl-rpm-bat" class="text-amber-500">0%</b></div>
                </div>
            </div>

            <!-- Fan Control Box (Tabs: [ระดับ] [อัตโนมัติ] [โปรแกรม]) (5 Cols ~40%) -->
            <div class="lg:col-span-5 glass-card rounded-2xl p-4 sm:p-5 space-y-4 transition-colors">
                
                <!-- Main Control Box Header & Tabs -->
                <div class="flex items-center justify-between border-b border-slate-200 dark:border-slate-800 pb-2.5">
                    <div class="flex items-center space-x-1">
                        <button id="ctrl-tab-lvl" onclick="switchControlTab('lvl')" class="px-3 py-1.5 rounded-xl text-xs font-bold bg-brand-50 text-brand-600 dark:bg-brand-950/60 dark:text-brand-400 transition-all">
                            ระดับ (Manual)
                        </button>
                        <button id="ctrl-tab-auto" onclick="switchControlTab('auto')" class="px-3 py-1.5 rounded-xl text-xs font-semibold text-slate-500 hover:text-slate-800 dark:hover:text-slate-200 transition-all">
                            อัตโนมัติ (Auto)
                        </button>
                        <button id="ctrl-tab-prog" onclick="switchControlTab('prog')" class="px-3 py-1.5 rounded-xl text-xs font-semibold text-slate-500 hover:text-slate-800 dark:hover:text-slate-200 transition-all">
                            โปรแกรม
                        </button>
                    </div>
                    <span id="ctrl-box-status" class="text-[11px] font-bold text-brand-600 dark:text-brand-400">
                        OFF (0%)
                    </span>
                </div>

                <!-- Tab 1: ระดับ (Keypad 0-5 & Collapsible Fine Tuning) -->
                <div id="pane-ctrl-lvl" class="space-y-4">
                    <!-- Keypad 3x2 Grid (Each button >= 56x56 px for gloves) -->
                    <div class="grid grid-cols-3 gap-2" id="level-keypad">
                        <button class="btn-level btn-touch-active min-h-[58px] rounded-xl border-2 border-slate-300 dark:border-slate-700 bg-slate-700 text-white font-bold flex flex-col items-center justify-center p-1 transition-all" data-lvl="0">
                            <span class="text-lg font-black leading-tight">OFF</span>
                            <span class="text-[10px] opacity-80">ปิดพัดลม</span>
                        </button>
                        <button class="btn-level btn-touch-active min-h-[58px] rounded-xl border-2 border-slate-200 dark:border-slate-700 bg-white dark:bg-slate-800 text-slate-800 dark:text-slate-100 font-bold flex flex-col items-center justify-center p-1 transition-all" data-lvl="1">
                            <span class="text-xl font-black leading-tight">1</span>
                            <span class="text-[10px] text-slate-500 dark:text-slate-400">20%</span>
                        </button>
                        <button class="btn-level btn-touch-active min-h-[58px] rounded-xl border-2 border-slate-200 dark:border-slate-700 bg-white dark:bg-slate-800 text-slate-800 dark:text-slate-100 font-bold flex flex-col items-center justify-center p-1 transition-all" data-lvl="2">
                            <span class="text-xl font-black leading-tight">2</span>
                            <span class="text-[10px] text-slate-500 dark:text-slate-400">40%</span>
                        </button>
                        <button class="btn-level btn-touch-active min-h-[58px] rounded-xl border-2 border-slate-200 dark:border-slate-700 bg-white dark:bg-slate-800 text-slate-800 dark:text-slate-100 font-bold flex flex-col items-center justify-center p-1 transition-all" data-lvl="3">
                            <span class="text-xl font-black leading-tight">3</span>
                            <span class="text-[10px] text-slate-500 dark:text-slate-400">60%</span>
                        </button>
                        <button class="btn-level btn-touch-active min-h-[58px] rounded-xl border-2 border-slate-200 dark:border-slate-700 bg-white dark:bg-slate-800 text-slate-800 dark:text-slate-100 font-bold flex flex-col items-center justify-center p-1 transition-all" data-lvl="4">
                            <span class="text-xl font-black leading-tight">4</span>
                            <span class="text-[10px] text-slate-500 dark:text-slate-400">80%</span>
                        </button>
                        <button class="btn-level btn-touch-active min-h-[58px] rounded-xl border-2 border-slate-200 dark:border-slate-700 bg-white dark:bg-slate-800 text-slate-800 dark:text-slate-100 font-bold flex flex-col items-center justify-center p-1 transition-all" data-lvl="5">
                            <span class="text-xl font-black leading-tight">5</span>
                            <span class="text-[10px] text-slate-500 dark:text-slate-400">100%</span>
                        </button>
                    </div>

                    <!-- Collapsible Fine Tuning Sliders -->
                    <details class="text-xs pt-1 border-t border-slate-100 dark:border-slate-800">
                        <summary class="font-bold text-slate-700 dark:text-slate-300 cursor-pointer py-1 flex items-center justify-between">
                            <span>💨 ปรับละเอียดแยกตัว/บ่า</span>
                            <span class="text-brand-600 dark:text-brand-400 text-[11px]">เปิดสไลเดอร์ ▼</span>
                        </summary>
                        <div class="space-y-3 pt-2">
                            <!-- Body Slider -->
                            <div>
                                <div class="flex justify-between items-center text-xs mb-1">
                                    <span class="font-semibold text-slate-700 dark:text-slate-300">พัดลมตัว 70mm (0–100%)</span>
                                    <span id="disp-slider-b" class="font-bold text-brand-600 dark:text-brand-400">0%</span>
                                </div>
                                <input id="slider-b" type="range" min="0" max="100" value="0" class="w-full h-2 bg-slate-200 dark:bg-slate-700 rounded-lg appearance-none cursor-pointer accent-brand-600" oninput="handleSliderB(this.value)">
                            </div>
                            <!-- Neck Slider (Capped <= 70%) -->
                            <div>
                                <div class="flex justify-between items-center text-xs mb-1">
                                    <span class="font-semibold text-slate-700 dark:text-slate-300">พัดลมบ่า 40mm (จำกัด ≤ 70%)</span>
                                    <span id="disp-slider-n" class="font-bold text-brand-600 dark:text-brand-400">0%</span>
                                </div>
                                <input id="slider-n" type="range" min="0" max="70" value="0" class="w-full h-2 bg-slate-200 dark:bg-slate-700 rounded-lg appearance-none cursor-pointer accent-brand-600" oninput="handleSliderN(this.value)">
                            </div>
                            <div class="p-2 rounded-xl bg-slate-50 dark:bg-slate-800/60 text-[11px] flex justify-between font-medium">
                                <span>ตัวจริง: <b id="ctrl-tele-b">0%</b></span>
                                <span>บ่าจริง: <b id="ctrl-tele-n">0%</b></span>
                                <span>แบตจริง: <b id="ctrl-tele-bat">0%</b></span>
                            </div>
                        </div>
                    </details>
                </div>

                <!-- Tab 2: อัตโนมัติ (Auto Mode & Presets) -->
                <div id="pane-ctrl-auto" class="space-y-3.5 hidden">
                    <!-- Auto Mode Master Switch -->
                    <div class="flex items-center justify-between p-3 rounded-xl bg-slate-50 dark:bg-slate-800/60 border border-slate-200 dark:border-slate-700">
                        <div>
                            <div class="text-xs font-bold text-slate-900 dark:text-white flex items-center gap-1.5">
                                <i data-lucide="bot" class="w-4 h-4 text-brand-600 dark:text-brand-400"></i>
                                <span>ระบบระบายความร้อนอัตโนมัติ</span>
                            </div>
                            <span id="auto-status-desc" class="text-[11px] text-slate-500 dark:text-slate-400">ปรับระดับพัดลมตามดัชนีความร้อน (HI)</span>
                        </div>
                        <label class="relative inline-flex items-center cursor-pointer">
                            <input type="checkbox" id="auto-master-switch" class="sr-only peer" onchange="toggleAutoMaster(this.checked)">
                            <div class="w-11 h-6 bg-slate-200 peer-focus:outline-none rounded-full peer dark:bg-slate-700 peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:border-slate-300 after:border after:rounded-full after:h-5 after:w-5 after:transition-all dark:border-slate-600 peer-checked:bg-brand-600"></div>
                        </label>
                    </div>

                    <!-- Auto Pause Alert & Resume Button -->
                    <div id="auto-paused-card" class="hidden p-3 rounded-xl bg-amber-50 dark:bg-amber-950/40 border border-amber-300 dark:border-amber-800 text-xs text-amber-900 dark:text-amber-200 space-y-2">
                        <div class="flex items-center justify-between font-bold">
                            <span class="flex items-center gap-1.5"><i data-lucide="pause-circle" class="w-4 h-4 text-amber-600"></i> Auto หยุดชั่วคราว</span>
                            <span id="auto-pause-reason" class="text-[10px] px-2 py-0.5 rounded bg-amber-200 dark:bg-amber-900 text-amber-800 dark:text-amber-200">ผู้ใช้สั่งหยุด</span>
                        </div>
                        <p class="text-[11px] text-slate-600 dark:text-slate-300">เมื่อพร้อมกลับสู่โหมดอัตโนมัติ กรุณาแตะปุ่มด้านล่าง:</p>
                        <button onclick="resumeAutoMode()" class="w-full py-2 px-3 rounded-lg bg-amber-600 hover:bg-amber-700 text-white font-bold text-xs flex items-center justify-center gap-1.5 transition-all">
                            <i data-lucide="play" class="w-3.5 h-3.5"></i> กลับไปทำงานอัตโนมัติ
                        </button>
                    </div>

                    <!-- Active Preset Selection -->
                    <div>
                        <div class="flex items-center justify-between text-xs font-bold text-slate-700 dark:text-slate-300 mb-1.5">
                            <span>พรีเซ็ตการทำงาน (เลือก 1 อัน):</span>
                            <button onclick="openCreatePresetModal()" class="text-brand-600 dark:text-brand-400 hover:underline text-[11px] font-semibold">+ สร้างพรีเซ็ต</button>
                        </div>
                        <div id="presets-list" class="space-y-1.5 max-h-48 overflow-y-auto pr-1">
                            <!-- Populated by JS -->
                        </div>
                    </div>

                    <!-- Calibration Note for Presets -->
                    <div class="text-[10px] text-slate-400 dark:text-slate-500 bg-slate-50 dark:bg-slate-800/40 p-2 rounded-lg border border-slate-100 dark:border-slate-800">
                        * พรีเซ็ตกลางแจ้ง/ในร่ม เป็นค่าสมมติฐานเริ่มต้น ต้องสอบเทียบ (Calibrate) จากการวัดจริง
                    </div>
                </div>

                <!-- Tab 3: โปรแกรม (Multi-phase Programs) -->
                <div id="pane-ctrl-prog" class="space-y-3 hidden">
                    <div class="flex items-center justify-between p-3 rounded-xl bg-slate-50 dark:bg-slate-800/60 border border-slate-200/80 dark:border-slate-700/80">
                        <div>
                            <div id="prog-active-title" class="text-xs font-bold text-slate-800 dark:text-white">ไม่ได้รันโปรแกรม</div>
                            <span class="text-[10px] text-slate-400">เลือกรันโปรแกรมด้านล่าง</span>
                        </div>
                        <div id="prog-countdown-timer" class="text-base font-black text-brand-600 dark:text-brand-400 font-mono">--:--</div>
                    </div>

                    <div class="grid grid-cols-2 gap-2">
                        <button id="btn-prog-0" onclick="startProgram(0)" class="btn-touch-active py-2.5 px-3 rounded-xl border border-slate-200 dark:border-slate-700 bg-white dark:bg-slate-800 hover:bg-slate-50 dark:hover:bg-slate-700 text-xs font-bold text-slate-800 dark:text-slate-200 transition-all">
                            ▶ Pre-cool
                        </button>
                        <button id="btn-prog-1" onclick="startProgram(1)" class="btn-touch-active py-2.5 px-3 rounded-xl border border-slate-200 dark:border-slate-700 bg-white dark:bg-slate-800 hover:bg-slate-50 dark:hover:bg-slate-700 text-xs font-bold text-slate-800 dark:text-slate-200 transition-all">
                            ▶ Ramp-up
                        </button>
                        <button id="btn-prog-2" onclick="startProgram(2)" class="btn-touch-active col-span-2 py-2 px-3 rounded-xl border border-slate-200 dark:border-slate-700 bg-white dark:bg-slate-800 hover:bg-slate-50 dark:hover:bg-slate-700 text-xs font-bold text-slate-800 dark:text-slate-200 transition-all">
                            ▶ Custom (ตามกำหนด 4 เฟส)
                        </button>
                        <button onclick="sendCmd(8, 0)" class="btn-touch-active col-span-2 py-1.5 px-3 rounded-xl bg-slate-100 hover:bg-slate-200 dark:bg-slate-800 text-[11px] font-semibold text-slate-500 dark:text-slate-400 transition-all">
                            ⏹ หยุดโปรแกรมอัตโนมัติ
                        </button>
                    </div>

                    <!-- Custom Program Accordion -->
                    <details class="text-xs pt-1 border-t border-slate-100 dark:border-slate-800">
                        <summary class="font-bold text-slate-700 dark:text-slate-300 cursor-pointer py-1">⚙️ ปรับแต่งตาราง Custom (บันทึก NVS)</summary>
                        <table class="w-full text-center mt-2 border-collapse text-[11px]">
                            <thead>
                                <tr class="text-slate-400 border-b border-slate-200 dark:border-slate-700">
                                    <th class="py-1">เฟส</th>
                                    <th>ระดับ</th>
                                    <th>นาที</th>
                                </tr>
                            </thead>
                            <tbody id="custom-table-body">
                                <!-- Populated by JS -->
                            </tbody>
                        </table>
                        <button onclick="saveCustomProgram()" class="mt-2.5 w-full py-2 rounded-xl bg-brand-600 hover:bg-brand-700 text-white font-bold text-xs transition-all">
                            💾 บันทึก Custom ลงเสื้อกั๊ก
                        </button>
                    </details>
                </div>

            </div>
        </div>

        <!-- Row 3: 5 Vital Metric Cards (Varied Size by Importance) & Advice Box -->
        <div class="space-y-3 sm:space-y-4">
            
            <!-- Contextual Assistant Advice Card (Non-Medical) -->
            <div id="advice-box" class="glass-card rounded-2xl p-3 sm:p-3.5 border-l-4 border-l-brand-500 flex items-start gap-2.5 transition-colors">
                <span id="advice-icon" class="text-lg shrink-0 mt-0.5">💡</span>
                <div class="flex-1 min-w-0">
                    <div class="text-[10px] font-bold text-slate-400 uppercase tracking-wider">คำแนะนำการใช้งาน</div>
                    <p id="advice-text" class="text-xs sm:text-[13px] font-medium text-slate-700 dark:text-slate-200 leading-snug">
                        เริ่มต้นใช้งาน: แตะเชื่อมต่อ BLE → เลือกระดับความเย็น 1–5 → แตะ STOP เมื่อต้องการหยุด
                    </p>
                </div>
                <button onclick="dismissAdvice()" class="text-slate-400 hover:text-slate-600 dark:hover:text-slate-200 text-sm font-bold px-1" title="ปิดคำแนะนำนี้">✕</button>
            </div>

            <!-- 5 Vital Metric Cards (HI & Ta largest) -->
            <div class="grid grid-cols-2 md:grid-cols-6 lg:grid-cols-12 gap-2.5 sm:gap-3 items-stretch">
                
                <!-- Card 1: Heat Index (HI) - LARGEST (col-span-2 md:col-span-3 lg:col-span-4) -->
                <div class="col-span-2 md:col-span-3 lg:col-span-4 glass-card rounded-2xl p-4 bg-gradient-to-br from-white to-amber-50/40 dark:from-slate-900 dark:to-amber-950/20 border-amber-200 dark:border-amber-900/60 flex flex-col justify-between">
                    <div class="flex items-center justify-between text-slate-500 dark:text-slate-400">
                        <span class="text-xs sm:text-sm font-bold flex items-center gap-1.5 text-amber-600 dark:text-amber-400">
                            ☀️ ดัชนีความร้อน (Heat Index)
                        </span>
                        <span class="sim-badge text-[9px] font-bold px-1.5 py-0.5 rounded bg-slate-100 dark:bg-slate-800 text-slate-500 dark:text-slate-400">จำลอง</span>
                    </div>
                    <div class="my-2 flex items-baseline gap-1.5">
                        <span id="val-hi" class="text-3xl sm:text-4xl font-black text-amber-600 dark:text-amber-400 font-mono">--.-</span>
                        <span class="text-sm font-bold text-slate-400">°C</span>
                        <span id="hi-risk-tag" class="ml-2 text-[10px] font-bold px-2 py-0.5 rounded-full bg-emerald-100 text-emerald-800 dark:bg-emerald-950/60 dark:text-emerald-300">ปกติ</span>
                    </div>
                    <p class="text-[11px] text-slate-500 dark:text-slate-400 leading-tight">คำนวณจากอุณหภูมิและความชื้นรอบตัว (สูตร Rothfusz)</p>
                </div>

                <!-- Card 2: Air Temp (Ta) - LARGE (col-span-1 md:col-span-3 lg:col-span-3) -->
                <div class="col-span-1 md:col-span-3 lg:col-span-3 glass-card rounded-2xl p-3.5 flex flex-col justify-between">
                    <div class="flex items-center justify-between text-slate-500 dark:text-slate-400">
                        <span class="text-[11px] font-semibold flex items-center gap-1">🌡️ อากาศ</span>
                        <span class="sim-badge text-[9px] font-bold px-1.5 py-0.5 rounded bg-slate-100 dark:bg-slate-800 text-slate-500 dark:text-slate-400">จำลอง</span>
                    </div>
                    <div class="my-1.5 flex items-baseline gap-1">
                        <span id="val-ta" class="text-2xl sm:text-3xl font-extrabold text-slate-900 dark:text-white font-mono">--.-</span>
                        <span class="text-xs font-semibold text-slate-400">°C</span>
                    </div>
                    <span class="text-[10px] text-slate-400">เซนเซอร์แวดล้อม</span>
                </div>

                <!-- Card 3: Air Humidity (Rh) (col-span-1 md:col-span-2 lg:col-span-2) -->
                <div class="col-span-1 md:col-span-2 lg:col-span-2 glass-card rounded-2xl p-3.5 flex flex-col justify-between">
                    <div class="flex items-center justify-between text-slate-500 dark:text-slate-400">
                        <span class="text-[11px] font-semibold flex items-center gap-1">💧 ความชื้น</span>
                        <span class="sim-badge text-[9px] font-bold px-1.5 py-0.5 rounded bg-slate-100 dark:bg-slate-800 text-slate-500 dark:text-slate-400">จำลอง</span>
                    </div>
                    <div class="my-1.5 flex items-baseline gap-1">
                        <span id="val-rh" class="text-2xl font-extrabold text-slate-900 dark:text-white font-mono">--</span>
                        <span class="text-xs font-semibold text-slate-400">%</span>
                    </div>
                    <span class="text-[10px] text-slate-400">ความชื้นสัมพัทธ์</span>
                </div>

                <!-- Card 4: Battery Temp (Tb) (col-span-1 md:col-span-2 lg:col-span-2) -->
                <div id="card-tb" class="col-span-1 md:col-span-2 lg:col-span-2 glass-card rounded-2xl p-3.5 flex flex-col justify-between">
                    <div class="flex items-center justify-between text-slate-500 dark:text-slate-400">
                        <span class="text-[11px] font-semibold flex items-center gap-1">🔋 แบต NTC</span>
                        <span class="sim-badge text-[9px] font-bold px-1.5 py-0.5 rounded bg-slate-100 dark:bg-slate-800 text-slate-500 dark:text-slate-400">จำลอง</span>
                    </div>
                    <div class="my-1.5 flex items-baseline gap-1">
                        <span id="val-tb" class="text-2xl font-extrabold text-slate-900 dark:text-white font-mono">--.-</span>
                        <span class="text-xs font-semibold text-slate-400">°C</span>
                    </div>
                    <span class="text-[10px] text-slate-400">ความร้อนก้อนแบต</span>
                </div>

                <!-- Card 5: Battery SOC (col-span-1 md:col-span-2 lg:col-span-1) -->
                <div id="card-soc" class="col-span-1 md:col-span-2 lg:col-span-1 glass-card rounded-2xl p-3.5 flex flex-col justify-between">
                    <div class="flex items-center justify-between text-slate-500 dark:text-slate-400">
                        <span class="text-[11px] font-semibold flex items-center gap-1">⚡ SOC</span>
                        <span class="sim-badge text-[9px] font-bold px-1.5 py-0.5 rounded bg-slate-100 dark:bg-slate-800 text-slate-500 dark:text-slate-400">จำลอง</span>
                    </div>
                    <div class="my-1.5 flex items-baseline gap-0.5">
                        <span id="val-soc" class="text-2xl font-extrabold text-slate-900 dark:text-white font-mono">--</span>
                        <span class="text-[10px] font-semibold text-slate-400">%</span>
                    </div>
                    <span class="text-[10px] text-slate-400">พลังงาน</span>
                </div>

            </div>

            <!-- Simulation Note -->
            <div id="sim-sensors-note" class="text-[11px] text-slate-400 dark:text-slate-500 text-center">
                * ข้อมูลทั้งหมดข้างต้นเป็นค่าจำลองสำหรับทดสอบระบบ เนื่องจากยังไม่ได้ต่อเซนเซอร์วัดค่าจริง
            </div>
        </div>

        <!-- Row 4: Telemetry Trend Graph (Chart.js) -->
        <div class="glass-card rounded-2xl p-4 sm:p-5 transition-colors">
            <div class="flex items-center justify-between mb-3">
                <div>
                    <h2 class="text-xs sm:text-sm font-bold text-slate-800 dark:text-white flex items-center gap-2">
                        <i data-lucide="line-chart" class="w-4 h-4 text-brand-600 dark:text-brand-400"></i>
                        แนวโน้มอุณหภูมิอากาศและแบตเตอรี่เรียลไทม์
                    </h2>
                    <p class="text-[11px] text-slate-500 dark:text-slate-400">พล็อตข้อมูลต่อเนื่องทุก 2 วินาที</p>
                </div>
                <div class="flex items-center gap-3 text-[11px] font-semibold">
                    <span class="flex items-center gap-1 text-sky-500"><span class="w-2.5 h-2.5 rounded-full bg-sky-500"></span> อากาศ</span>
                    <span class="flex items-center gap-1 text-amber-500"><span class="w-2.5 h-2.5 rounded-full bg-amber-500"></span> แบต NTC</span>
                </div>
            </div>
            <div class="h-44 sm:h-52 w-full relative">
                <canvas id="telemetryChart"></canvas>
            </div>
        </div>

    </main>

    <!-- Sticky Bottom Emergency STOP Button (Always Accessible, Not in any tab) -->
    <div class="fixed bottom-3 left-0 right-0 max-w-xl mx-auto px-3 sm:px-4 z-30 pointer-events-none">
        <button id="btn-stop" onclick="handleEmergencyStop()" class="btn-touch-active pointer-events-auto w-full min-h-[58px] rounded-2xl bg-rose-600 hover:bg-rose-700 text-white font-black text-base sm:text-lg flex items-center justify-center gap-2.5 shadow-2xl shadow-rose-600/40 border-2 border-rose-500/80 transition-all">
            <i data-lucide="octagon-alert" class="w-6 h-6"></i>
            <span>⛔ STOP หยุดฉุกเฉิน (ดับพัดลมทันที)</span>
        </button>
    </div>

    <!-- Modal: Equipment List BOM -->
    <div id="equipment-modal" class="fixed inset-0 z-50 flex items-center justify-center p-2 sm:p-4 bg-slate-900/60 backdrop-blur-sm hidden transition-opacity">
        <div class="bg-white dark:bg-slate-900 border border-slate-200 dark:border-slate-800 rounded-2xl shadow-2xl max-w-4xl w-full max-h-[92vh] flex flex-col overflow-hidden">
            <div class="flex items-center justify-between px-4 sm:px-6 py-3.5 border-b border-slate-200 dark:border-slate-800">
                <div class="flex items-center space-x-2">
                    <div class="w-8 h-8 rounded-lg bg-brand-50 dark:bg-brand-950/50 text-brand-600 dark:text-brand-400 flex items-center justify-center shrink-0">
                        <i data-lucide="cpu" class="w-4 h-4"></i>
                    </div>
                    <div>
                        <h3 class="text-sm sm:text-base font-bold text-slate-900 dark:text-white leading-tight">รายการอุปกรณ์และประมาณการงบประมาณ (BOM List)</h3>
                        <p class="text-[11px] text-slate-500 dark:text-slate-400">ระบบเสื้อกั๊กระบายความร้อนอัจฉริยะ (Smart Cooling Vest)</p>
                    </div>
                </div>
                <button onclick="toggleEquipmentModal(false)" class="p-1.5 rounded-xl text-slate-400 hover:text-slate-700 dark:hover:text-slate-200 hover:bg-slate-100 dark:hover:bg-slate-800 transition">
                    <i data-lucide="x" class="w-5 h-5"></i>
                </button>
            </div>
            <div class="p-3 sm:p-6 overflow-y-auto flex-1 space-y-4">
                <div class="overflow-x-auto rounded-xl border border-slate-200 dark:border-slate-800">
                    <table class="w-full text-xs text-left text-slate-700 dark:text-slate-300 min-w-[640px]">
                        <thead class="text-xs uppercase bg-slate-50 dark:bg-slate-800/80 text-slate-500 dark:text-slate-400 font-semibold border-b border-slate-200 dark:border-slate-700">
                            <tr>
                                <th class="px-3 py-3 text-center w-12">ลำดับ</th>
                                <th class="px-4 py-3">รายการอุปกรณ์</th>
                                <th class="px-4 py-3">รุ่น / สเปกทางเทคนิค</th>
                                <th class="px-4 py-3">หน้าที่ในระบบ</th>
                                <th class="px-3 py-3 text-center w-16">จำนวน</th>
                                <th class="px-4 py-3 text-right">ราคาประเมิน</th>
                            </tr>
                        </thead>
                        <tbody class="divide-y divide-slate-100 dark:divide-slate-800/60 font-medium">
                            <tr class="hover:bg-slate-50/50 dark:hover:bg-slate-800/30">
                                <td class="px-3 py-2.5 text-center text-slate-400">1</td>
                                <td class="px-4 py-2.5 font-semibold text-slate-900 dark:text-white">ไมโครคอนโทรลเลอร์</td>
                                <td class="px-4 py-2.5 text-slate-500 dark:text-slate-400">ESP32-S3 (Dual-Core 240MHz, BLE 5.0, USB-CDC)</td>
                                <td class="px-4 py-2.5">ประมวลผลตรรกะความปลอดภัย, PWM 25kHz, ส่ง Telemetry</td>
                                <td class="px-3 py-2.5 text-center">1 บอร์ด</td>
                                <td class="px-4 py-2.5 text-right font-semibold text-brand-600 dark:text-brand-400">190.-</td>
                            </tr>
                            <tr class="hover:bg-slate-50/50 dark:hover:bg-slate-800/30">
                                <td class="px-3 py-2.5 text-center text-slate-400">2</td>
                                <td class="px-4 py-2.5 font-semibold text-slate-900 dark:text-white">พัดลมระบายความร้อนตัว (ช่อง A/B)</td>
                                <td class="px-4 py-2.5 text-slate-500 dark:text-slate-400">พัดลมหอยโข่ง 70 mm (×2) 5400 RPM</td>
                                <td class="px-4 py-2.5">เป่าระบายความร้อนลำตัวและแผ่นหลัง</td>
                                <td class="px-3 py-2.5 text-center">2 ตัว</td>
                                <td class="px-4 py-2.5 text-right font-semibold text-brand-600 dark:text-brand-400">380.-</td>
                            </tr>
                            <tr class="hover:bg-slate-50/50 dark:hover:bg-slate-800/30">
                                <td class="px-3 py-2.5 text-center text-slate-400">3</td>
                                <td class="px-4 py-2.5 font-semibold text-slate-900 dark:text-white">พัดลมระบายความร้อนบ่า/คอ</td>
                                <td class="px-4 py-2.5 text-slate-500 dark:text-slate-400">ไมโครโบลเวอร์ 40 mm (×2) องศาเป่า 45° (≤ 70%)</td>
                                <td class="px-4 py-2.5">เป่าระบายเหงื่อต้นคอและใบหน้าแบบแฮนด์ฟรี</td>
                                <td class="px-3 py-2.5 text-center">2 ตัว</td>
                                <td class="px-4 py-2.5 text-right font-semibold text-brand-600 dark:text-brand-400">220.-</td>
                            </tr>
                            <tr class="hover:bg-slate-50/50 dark:hover:bg-slate-800/30">
                                <td class="px-3 py-2.5 text-center text-slate-400">4</td>
                                <td class="px-4 py-2.5 font-semibold text-slate-900 dark:text-white">พัดลมระบายความร้อนแบตเตอรี่ (BTMS)</td>
                                <td class="px-4 py-2.5 text-slate-500 dark:text-slate-400">Micro Fan 30 mm + NTC 10k แนบแพ็กแบตเตอรี่</td>
                                <td class="px-4 py-2.5">เปิดอัตโนมัติเมื่อแบตเตอรี่ ≥40°C ป้องกันบวม</td>
                                <td class="px-3 py-2.5 text-center">1 ตัว</td>
                                <td class="px-4 py-2.5 text-right font-semibold text-brand-600 dark:text-brand-400">120.-</td>
                            </tr>
                            <tr class="hover:bg-slate-50/50 dark:hover:bg-slate-800/30">
                                <td class="px-3 py-2.5 text-center text-slate-400">5</td>
                                <td class="px-4 py-2.5 font-semibold text-slate-900 dark:text-white">โมดูลขับกำลัง MOSFET / ไดรฟ์</td>
                                <td class="px-4 py-2.5 text-slate-500 dark:text-slate-400">DRV8833 Dual Driver หรือ Logic-Level MOSFET Module</td>
                                <td class="px-4 py-2.5">ขับสัญญาณ PWM 25kHz แยก 3 แชนเนลอิสระ</td>
                                <td class="px-3 py-2.5 text-center">2 ชุด</td>
                                <td class="px-4 py-2.5 text-right font-semibold text-brand-600 dark:text-brand-400">140.-</td>
                            </tr>
                            <tr class="hover:bg-slate-50/50 dark:hover:bg-slate-800/30">
                                <td class="px-3 py-2.5 text-center text-slate-400">6</td>
                                <td class="px-4 py-2.5 font-semibold text-slate-900 dark:text-white">แบตเตอรี่ & วงจร BMS</td>
                                <td class="px-4 py-2.5 text-slate-500 dark:text-slate-400">2S2P Li-ion 18650 Pack 7.4V + Hardware BMS 2S + ฟิวส์</td>
                                <td class="px-4 py-2.5">แหล่งพลังงานหลักพร้อมระบบตัดวงจรนิรภัย 60°C</td>
                                <td class="px-3 py-2.5 text-center">1 ชุด</td>
                                <td class="px-4 py-2.5 text-right font-semibold text-brand-600 dark:text-brand-400">520.-</td>
                            </tr>
                        </tbody>
                    </table>
                </div>
            </div>
        </div>
    </div>

    <!-- Modal: Quick Guide '?' -->
    <div id="help-modal" class="fixed inset-0 z-50 flex items-center justify-center p-3 bg-slate-900/60 backdrop-blur-sm hidden">
        <div class="bg-white dark:bg-slate-900 border border-slate-200 dark:border-slate-800 rounded-2xl shadow-2xl max-w-md w-full p-5 space-y-4">
            <div class="flex items-center justify-between border-b border-slate-100 dark:border-slate-800 pb-2.5">
                <h3 class="font-bold text-slate-900 dark:text-white flex items-center gap-2">
                    📖 คู่มือการใช้งานย่อ
                </h3>
                <button onclick="toggleHelpModal(false)" class="text-slate-400 hover:text-slate-700 dark:hover:text-slate-200 text-lg font-bold">✕</button>
            </div>
            <div class="space-y-3 text-xs text-slate-600 dark:text-slate-300 leading-relaxed max-h-[70vh] overflow-y-auto">
                <div>
                    <b class="text-brand-600 dark:text-brand-400">ความหมายของระดับความแรง:</b>
                    <p>• OFF = ปิดพัดลม<br>• 1 = 20% ลมเบาสบาย<br>• 2 = 40% ทำงานทั่วไป<br>• 3 = 60% ลมแรงระบายเหงื่อ<br>• 4 = 80% เย็นเร็ว<br>• 5 = 100% สูงสุด (พัดลมบ่าจำกัดที่ 70% เสมอ)</p>
                </div>
                <div>
                    <b class="text-brand-600 dark:text-brand-400">ความหมายของสีสถานะ:</b>
                    <p>• 🟢 เขียว = อุณหภูมิและแบตเตอรี่ปกติ<br>• 🟡 เหลือง = แบตร้อนขึ้น (≥40°C) พัดลมแบตทำงาน หรือแบตเหลือ &lt;20%<br>• 🟠 ส้ม = แบตร้อน 50°C ระบบลดกำลังพัดลมเหลือ 60% อัตโนมัติ<br>• 🔴 แดง = หยุดฉุกเฉิน หรือตัดวงจรเนื่องจากแบตร้อนเกิน 60°C</p>
                </div>
                <div>
                    <b class="text-brand-600 dark:text-brand-400">วิธีรีเซ็ตระบบเมื่อถูกตัดวงจร:</b>
                    <p>หากระบบตัดไฟเนื่องจากแบตร้อนเกิน 60°C ให้รอแบตเตอรี่เย็นลงแล้วแตะปุ่ม "รีเซ็ตระบบ" บนการ์ดสถานะเพื่อใช้งานต่อ</p>
                </div>
            </div>
        </div>
    </div>

    <!-- Modal: Developer & Testing Tools -->
    <div id="dev-modal" class="fixed inset-0 z-50 flex items-center justify-center p-3 bg-slate-900/60 backdrop-blur-sm hidden">
        <div class="bg-white dark:bg-slate-900 border border-slate-200 dark:border-slate-800 rounded-2xl shadow-2xl max-w-md w-full p-5 space-y-4">
            <div class="flex items-center justify-between border-b border-slate-100 dark:border-slate-800 pb-2.5">
                <h3 class="font-bold text-slate-900 dark:text-white flex items-center gap-2">
                    <i data-lucide="wrench" class="w-4 h-4 text-amber-500"></i> เครื่องมือผู้พัฒนาและทดสอบ
                </h3>
                <button onclick="toggleDevModal(false)" class="text-slate-400 hover:text-slate-700 dark:hover:text-slate-200 text-lg font-bold">✕</button>
            </div>
            <div class="space-y-3 text-xs">
                <div class="flex items-center justify-between py-1 border-b border-slate-100 dark:border-slate-800">
                    <label for="chk-fast-test" class="font-semibold text-slate-700 dark:text-slate-300">⚡ โหมดทดสอบเร็ว (1 นาที = 6 วินาที)</label>
                    <input type="checkbox" id="chk-fast-test" onchange="sendCmd(12, this.checked ? 1 : 0); showToast(this.checked ? 'เปิดโหมดทดสอบเร็ว' : 'ปิดโหมดทดสอบเร็ว')">
                </div>
                <div class="grid grid-cols-2 gap-2">
                    <button onclick="sendCmd(6, 0); showToast('สลับจำลองแบตร้อน')" class="p-2.5 rounded-xl bg-slate-100 dark:bg-slate-800 hover:bg-slate-200 font-semibold text-slate-700 dark:text-slate-200">
                        🔥 จำลองแบตร้อน
                    </button>
                    <button onclick="sendCmd(5, 0); showToast('ส่งคำสั่งรีเซ็ต')" class="p-2.5 rounded-xl bg-slate-100 dark:bg-slate-800 hover:bg-slate-200 font-semibold text-slate-700 dark:text-slate-200">
                        🔄 ส่งรีเซ็ต
                    </button>
                    <button onclick="startWifiMode()" class="p-2.5 rounded-xl bg-slate-100 dark:bg-slate-800 hover:bg-slate-200 font-semibold text-slate-700 dark:text-slate-200">
                        📶 สลับ Wi-Fi AP
                    </button>
                    <button onclick="startDemoMode()" class="p-2.5 rounded-xl bg-slate-100 dark:bg-slate-800 hover:bg-slate-200 font-semibold text-slate-700 dark:text-slate-200">
                        🧪 โหมด Demo
                    </button>
                </div>
                <div id="raw-tele-box" class="p-2.5 rounded-lg bg-slate-100 dark:bg-slate-800 text-[10px] font-mono text-slate-500 break-all">
                    Telemetry Raw: รอข้อมูล...
                </div>
            </div>
        </div>
    </div>

    <!-- Modal: Create / Edit Custom Preset Dialog -->
    <div id="preset-modal" class="fixed inset-0 z-50 flex items-center justify-center p-3 bg-slate-900/60 backdrop-blur-sm hidden">
        <div class="bg-white dark:bg-slate-900 border border-slate-200 dark:border-slate-800 rounded-2xl shadow-2xl max-w-md w-full p-5 space-y-4 max-h-[90vh] overflow-y-auto">
            <div class="flex items-center justify-between border-b border-slate-100 dark:border-slate-800 pb-2">
                <h3 id="preset-modal-title" class="font-bold text-slate-900 dark:text-white">สร้างพรีเซ็ตอัตโนมัติ</h3>
                <button onclick="togglePresetModal(false)" class="text-slate-400 hover:text-slate-700 text-lg font-bold">✕</button>
            </div>
            
            <div class="space-y-3 text-xs">
                <div>
                    <label class="font-semibold block mb-1">ชื่อพรีเซ็ต:</label>
                    <input id="inp-preset-name" type="text" placeholder="เช่น ทำงานกะบ่าย" class="w-full p-2 border border-slate-200 dark:border-slate-700 rounded-lg bg-white dark:bg-slate-800">
                </div>
                <div class="grid grid-cols-2 gap-2">
                    <div>
                        <label class="font-semibold block mb-1">ไอคอน:</label>
                        <select id="sel-preset-icon" class="w-full p-2 border border-slate-200 dark:border-slate-700 rounded-lg bg-white dark:bg-slate-800">
                            <option value="⚡">⚡ ทั่วไป</option>
                            <option value="☀️">☀️ แดดแรง</option>
                            <option value="🏭">🏭 โรงงาน</option>
                            <option value="🏃">🏃 เคลื่อนไหวสูง</option>
                            <option value="🌙">🌙 กะดึก</option>
                        </select>
                    </div>
                    <div>
                        <label class="font-semibold block mb-1">ระดับสูงสุด (เพดาน):</label>
                        <select id="sel-preset-maxlvl" class="w-full p-2 border border-slate-200 dark:border-slate-700 rounded-lg bg-white dark:bg-slate-800">
                            <option value="5">ระดับ 5 (100%)</option>
                            <option value="4">ระดับ 4 (80%)</option>
                            <option value="3">ระดับ 3 (60%)</option>
                            <option value="2">ระดับ 2 (40%)</option>
                        </select>
                    </div>
                </div>

                <div>
                    <div class="flex items-center justify-between mb-1">
                        <label class="font-semibold">ช่วงดัชนีความร้อน HI (1–5 ช่วง):</label>
                        <button type="button" onclick="addPresetBandRow()" class="text-brand-600 dark:text-brand-400 font-bold text-[11px]">+ เพิ่มช่วง</button>
                    </div>
                    <div id="preset-bands-container" class="space-y-1.5">
                        <!-- Populated by JS -->
                    </div>
                </div>
            </div>

            <div class="flex justify-end gap-2 pt-2 border-t border-slate-100 dark:border-slate-800">
                <button onclick="togglePresetModal(false)" class="px-3 py-1.5 rounded-lg border border-slate-200 text-slate-600 text-xs font-semibold">ยกเลิก</button>
                <button onclick="savePresetForm()" class="px-4 py-1.5 rounded-lg bg-brand-600 text-white text-xs font-bold">บันทึกพรีเซ็ต</button>
            </div>
        </div>
    </div>

    <!-- Modal: In-Page Confirmation Dialog (Replaces confirm()) -->
    <div id="confirm-modal" class="fixed inset-0 z-50 flex items-center justify-center p-3 bg-slate-900/60 backdrop-blur-sm hidden">
        <div class="bg-white dark:bg-slate-900 border border-slate-200 dark:border-slate-800 rounded-2xl shadow-2xl max-w-sm w-full p-5 space-y-3">
            <h4 id="confirm-modal-title" class="font-bold text-slate-900 dark:text-white text-sm">ยืนยันการดำเนินการ</h4>
            <p id="confirm-modal-desc" class="text-xs text-slate-600 dark:text-slate-300">คุณต้องการดำเนินการนี้หรือไม่?</p>
            <div class="flex justify-end gap-2 pt-2">
                <button id="btn-confirm-cancel" class="px-3 py-1.5 rounded-lg border border-slate-200 text-slate-600 text-xs font-semibold">ยกเลิก</button>
                <button id="btn-confirm-ok" class="px-4 py-1.5 rounded-lg bg-rose-600 text-white text-xs font-bold">ยืนยัน</button>
            </div>
        </div>
    </div>

    <!-- Inline Toast Banner (Zero alert() calls) -->
    <div id="toast-banner" class="toast-banner"></div>

    <!-- Application Script (Logic, 3D Engine, Web BLE, Auto System, Protocol) -->
    <script>
        // --- Exact BLE GATT Protocol Constants ---
        const SVC_UUID = '7d1c0001-2f4b-4c1a-9a53-5b0e6f6c0a01';
        const CMD_UUID = '7d1c0002-2f4b-4c1a-9a53-5b0e6f6c0a01';
        const TEL_UUID = '7d1c0003-2f4b-4c1a-9a53-5b0e6f6c0a01';

        // Sensor simulation constant (Requirement C)
        const SIMULATED_SENSORS = true;

        const LV_PCT = [0, 20, 40, 60, 80, 100];
        const PROG_NAMES = ['Pre-cool', 'Ramp-up', 'Custom'];
        const PROG_CONFIGS = [
            [[5, 3], [3, 10], [2, 0]],
            [[1, 5], [2, 5], [3, 0]],
            [[5, 2], [3, 5], [1, 0], [255, 0]]
        ];

        // Global DOM Selectors
        const $ = s => document.querySelector(s);
        const $$ = s => document.querySelectorAll(s);

        // System State
        let bleDevice = null;
        let cmdChar = null;
        let telChar = null;
        let connMode = 'NONE'; // 'BLE', 'WIFI', 'DEMO'
        let writeQueue = Promise.resolve();
        let pollTimer = null;
        let activeLevel = 0;
        let currentProg = 0;
        let currentPhase = 0;
        let phaseStartTime = 0;
        let phaseDurationSec = 0;
        let countdownTimer = null;
        let userTappedStop = false;
        let currentTheme = localStorage.getItem('theme') || 'light';

        // Custom program local copy
        let localCustom = [
            { lvl: 5, min: 2 },
            { lvl: 3, min: 5 },
            { lvl: 1, min: 0 },
            { lvl: 255, min: 0 }
        ];

        const dismissedAdvice = new Set();
        let currentAdviceId = 'onboard';

        // --- Heat Index Formula (Rothfusz Regression Equation) ---
        function calculateHeatIndex(ta, rh) {
            if (ta < 27.0) return Math.round(ta * 10) / 10;
            const T = (ta * 9 / 5) + 32;
            const R = rh;
            let hi_f = -42.379 + (2.04901523 * T) + (10.14333127 * R)
                       - (0.22475541 * T * R) - (0.00683783 * T * T)
                       - (0.05481717 * R * R) + (0.00122874 * T * T * R)
                       + (0.00085282 * T * R * R) - (0.00000199 * T * T * R * R);

            if (R < 13 && T >= 80 && T <= 112) {
                hi_f -= ((13 - R) / 4) * Math.sqrt((17 - Math.abs(T - 95)) / 17);
            } else if (R > 85 && T >= 80 && T <= 87) {
                hi_f += ((R - 85) / 10) * ((87 - T) / 5);
            }
            const hi_c = (hi_f - 32) * 5 / 9;
            return Math.round(hi_c * 10) / 10;
        }

        // Haptic Feedback Helper
        function haptic(ms = 25) {
            if ('vibrate' in navigator) {
                try { navigator.vibrate(ms); } catch (e) {}
            }
        }

        // Inline Toast Notification (No alert() used)
        let toastTimer = null;
        function showToast(msg, dur = 3000) {
            const el = $('#toast-banner');
            el.textContent = msg;
            el.classList.add('show');
            if (toastTimer) clearTimeout(toastTimer);
            toastTimer = setTimeout(() => el.classList.remove('show'), dur);
        }

        // In-page Confirmation Dialog (Replaces confirm())
        function showConfirm(title, desc, onConfirm) {
            $('#confirm-modal-title').textContent = title;
            $('#confirm-modal-desc').textContent = desc;
            const modal = $('#confirm-modal');
            modal.classList.remove('hidden');

            const btnOk = $('#btn-confirm-ok');
            const btnCancel = $('#btn-confirm-cancel');

            const cleanup = () => {
                modal.classList.add('hidden');
                btnOk.onclick = null;
                btnCancel.onclick = null;
            };

            btnCancel.onclick = cleanup;
            btnOk.onclick = () => {
                cleanup();
                onConfirm();
            };
        }

        // --- BLE GATT Command Transmitter ---
        function sendCmd(cmd, val = 0, a = 0, b = 0) {
            haptic(25);
            if (cmd === 4) userTappedStop = true;
            if (cmd >= 1 && cmd <= 3) userTappedStop = false;

            if (connMode === 'BLE' && cmdChar) {
                writeQueue = writeQueue.then(() => {
                    return cmdChar.writeValue(Uint8Array.of(cmd, val, a, b));
                }).catch(err => console.warn('BLE write error:', err));
            } else if (connMode === 'WIFI') {
                fetch(`/api/cmd?c=${cmd}&v=${val}&a=${a}&b=${b}`).catch(e => console.warn(e));
            } else if (connMode === 'DEMO') {
                demoHandleCmd(cmd, val, a, b);
            }
        }

        // --- Telemetry 12-Byte Parser ---
        function parseTelemetry(view) {
            return {
                ta: view.getInt16(0, true) / 10,
                rh: view.getUint8(2),
                tb: view.getInt16(3, true) / 10,
                soc: view.getUint8(5),
                b: view.getUint8(6),
                n: view.getUint8(7),
                bat: view.getUint8(8),
                f: view.getUint8(9),      // bit0: cut/stopped, bit1: sim hot, bit2: derate
                ph: view.getUint8(10),    // 0 = none, 1..4 = active
                pg: view.getUint8(11)     // 0=Pre-cool, 1=Ramp-up, 2=Custom
            };
        }

        // Current telemetry cache
        let latestTel = { ta: 34.0, rh: 60, tb: 30.5, soc: 85, b: 0, n: 0, bat: 0, f: 0, ph: 0, pg: 0 };

        // --- Main UI Updater ---
        function updateTelemetryUI(d) {
            latestTel = d;
            const hi = calculateHeatIndex(d.ta, d.rh);

            // Numbers
            $('#val-ta').textContent = d.ta.toFixed(1);
            $('#val-rh').textContent = d.rh;
            $('#val-tb').textContent = d.tb.toFixed(1);
            $('#val-soc').textContent = d.soc;
            $('#val-hi').textContent = hi.toFixed(1);

            // HI Risk Tag
            const hiTag = $('#hi-risk-tag');
            if (hi >= 40.0) {
                hiTag.textContent = 'อันตรายสูง';
                hiTag.className = "ml-2 text-[10px] font-bold px-2 py-0.5 rounded-full bg-rose-100 text-rose-800 dark:bg-rose-950/60 dark:text-rose-300";
            } else if (hi >= 36.0) {
                hiTag.textContent = 'ระวังมาก';
                hiTag.className = "ml-2 text-[10px] font-bold px-2 py-0.5 rounded-full bg-amber-100 text-amber-800 dark:bg-amber-950/60 dark:text-amber-300";
            } else if (hi >= 32.0) {
                hiTag.textContent = 'เตือนระวัง';
                hiTag.className = "ml-2 text-[10px] font-bold px-2 py-0.5 rounded-full bg-yellow-100 text-yellow-800 dark:bg-yellow-950/60 dark:text-yellow-300";
            } else {
                hiTag.textContent = 'ปกติ';
                hiTag.className = "ml-2 text-[10px] font-bold px-2 py-0.5 rounded-full bg-emerald-100 text-emerald-800 dark:bg-emerald-950/60 dark:text-emerald-300";
            }

            // Sync 3D Real Fan Speeds & Badges
            liveSpeedBody = d.b;
            liveSpeedNeck = d.n;
            liveSpeedBat = d.bat;

            $('#3d-fan-badge').textContent = `${d.b}% (${Math.round(d.b * 54)} RPM)`;
            $('#lbl-rpm-body').textContent = `${d.b}% (${Math.round(d.b * 54)} RPM)`;
            $('#lbl-rpm-neck').textContent = `${d.n}% (${Math.round(d.n * 50)} RPM)`;
            $('#lbl-rpm-bat').textContent = `${d.bat}% (${Math.round(d.bat * 60)} RPM)`;

            // 2D fallback sync
            $('#fan-2d-body')?.style.setProperty('animation', d.b > 0 ? `spin ${Math.max(0.2, 2.5 - (d.b/100)*2.2)}s linear infinite` : 'none');
            $('#fan-2d-neck')?.style.setProperty('animation', d.n > 0 ? `spin ${Math.max(0.2, 2.5 - (d.n/100)*2.2)}s linear infinite` : 'none');
            $('#fan-2d-bat')?.style.setProperty('animation', d.bat > 0 ? `spin ${Math.max(0.2, 2.5 - (d.bat/100)*2.2)}s linear infinite` : 'none');

            // Sliders Telemetry Readout
            $('#ctrl-tele-b').textContent = d.b + '%';
            $('#ctrl-tele-n').textContent = d.n + '%';
            $('#ctrl-tele-bat').textContent = d.bat + '%';

            // Battery Temp & SOC warning card styles
            const cardTb = $('#card-tb');
            if (d.tb >= 50) cardTb.className = "col-span-1 md:col-span-2 lg:col-span-2 glass-card rounded-2xl p-3.5 flex flex-col justify-between border-rose-500 bg-rose-50/50 dark:bg-rose-950/20";
            else if (d.tb >= 40) cardTb.className = "col-span-1 md:col-span-2 lg:col-span-2 glass-card rounded-2xl p-3.5 flex flex-col justify-between border-amber-500 bg-amber-50/50 dark:bg-amber-950/20";
            else cardTb.className = "col-span-1 md:col-span-2 lg:col-span-2 glass-card rounded-2xl p-3.5 flex flex-col justify-between";

            const cardSoc = $('#card-soc');
            if (d.soc < 20) cardSoc.className = "col-span-1 md:col-span-2 lg:col-span-1 glass-card rounded-2xl p-3.5 flex flex-col justify-between border-amber-500 bg-amber-50/50 dark:bg-amber-950/20";
            else cardSoc.className = "col-span-1 md:col-span-2 lg:col-span-1 glass-card rounded-2xl p-3.5 flex flex-col justify-between";

            // Evaluate flags
            const isCutOrStopped = (d.f & 1) !== 0;
            const isSimHot = (d.f & 2) !== 0;
            const isDerated = (d.f & 4) !== 0;

            // Hero condition banner
            const heroCard = $('#hero-status-card');
            const heroIcon = $('#hero-icon');
            const heroTitle = $('#hero-title');
            const heroTag = $('#hero-tag');
            const heroDesc = $('#hero-desc');
            const btnReset = $('#btn-hero-reset');

            btnReset.classList.add('hidden');

            if (isCutOrStopped) {
                heroCard.className = "glass-card rounded-2xl p-4 sm:p-5 border-l-4 border-l-rose-500 bg-rose-50/40 dark:bg-rose-950/20";
                if (d.tb >= 60) {
                    heroIcon.textContent = '🔴';
                    heroTitle.textContent = 'ระบบตัดวงจรเพื่อความปลอดภัย';
                    heroTag.textContent = 'แบตร้อนเกิน 60°C';
                    heroTag.className = "text-[11px] font-bold px-2 py-0.5 rounded-full bg-rose-100 text-rose-700 dark:bg-rose-900/60 dark:text-rose-300";
                    heroDesc.textContent = 'วงจรตัดการจ่ายไฟค้างไว้เพื่อความปลอดภัยสูงสุด กรุณารอให้แบตเตอรี่ระบายความร้อนต่ำลง แล้วแตะปุ่มรีเซ็ต';
                } else {
                    heroIcon.textContent = '⛔';
                    heroTitle.textContent = 'พัดลมหยุดทำงาน (STOP)';
                    heroTag.textContent = 'หยุดสนิท';
                    heroTag.className = "text-[11px] font-bold px-2 py-0.5 rounded-full bg-rose-100 text-rose-700 dark:bg-rose-900/60 dark:text-rose-300";
                    heroDesc.textContent = 'พัดลมทุกตัวดับสนิท เลือกระดับความแรง 1–5 เมื่อพร้อมใช้งานต่อ';
                }
                btnReset.classList.remove('hidden');
                setActiveKeypadUI(0);
            } else if (isDerated || d.tb >= 50) {
                heroCard.className = "glass-card rounded-2xl p-4 sm:p-5 border-l-4 border-l-amber-500 bg-amber-50/40 dark:bg-amber-950/20";
                heroIcon.textContent = '🟠';
                heroTitle.textContent = `ลดโหลดอัตโนมัติ (แบต ${d.tb.toFixed(1)}°C)`;
                heroTag.textContent = 'ลดเหลือ 60%';
                heroTag.className = "text-[11px] font-bold px-2 py-0.5 rounded-full bg-amber-100 text-amber-800 dark:bg-amber-900/60 dark:text-amber-300";
                heroDesc.textContent = 'ระบบปรับลดกำลังพัดลมลงอัตโนมัติ เพื่อป้องกันความร้อนสะสมในแบตเตอรี่';
            } else if (d.tb >= 40 || d.soc < 20) {
                heroCard.className = "glass-card rounded-2xl p-4 sm:p-5 border-l-4 border-l-amber-400 bg-amber-50/30 dark:bg-amber-950/20";
                if (d.tb >= 40) {
                    heroIcon.textContent = '🟡';
                    heroTitle.textContent = `แบตร้อน ${d.tb.toFixed(1)}°C (พัดลมแบตทำงาน)`;
                    heroTag.textContent = 'ระบายความร้อนแบต';
                    heroTag.className = "text-[11px] font-bold px-2 py-0.5 rounded-full bg-amber-100 text-amber-800 dark:bg-amber-900/60 dark:text-amber-300";
                    heroDesc.textContent = 'พัดลมระบายความร้อนแบตเตอรี่กำลังทำงานเพื่อรักษาอุณหภูมิเซลล์';
                } else {
                    heroIcon.textContent = '🟡';
                    heroTitle.textContent = `แบตเตอรี่เหลือน้อย (${d.soc}%)`;
                    heroTag.textContent = 'เตรียมชาร์จ';
                    heroTag.className = "text-[11px] font-bold px-2 py-0.5 rounded-full bg-amber-100 text-amber-800 dark:bg-amber-900/60 dark:text-amber-300";
                    heroDesc.textContent = 'แบตเตอรี่เหลือต่ำกว่า 20% ควรเตรียมสลับก้อนใหม่หรือชาร์จไฟ';
                }
            } else {
                heroCard.className = "glass-card rounded-2xl p-4 sm:p-5 border-l-4 border-l-emerald-500";
                if (d.b > 0 || d.n > 0) {
                    heroIcon.textContent = '🟢';
                    heroTitle.textContent = 'เย็นสบาย · กำลังระบายอากาศ';
                    heroTag.textContent = 'ปกติ';
                    heroTag.className = "text-[11px] font-bold px-2 py-0.5 rounded-full bg-emerald-50 text-emerald-700 dark:bg-emerald-950/60 dark:text-emerald-400";
                    heroDesc.textContent = `พัดลมตัว ${d.b}% | พัดลมบ่า ${d.n}% | แบตเตอรี่ ${d.tb.toFixed(1)}°C ปกติ`;
                } else {
                    heroIcon.textContent = '🟢';
                    heroTitle.textContent = 'พร้อมใช้งาน (สแตนด์บาย)';
                    heroTag.textContent = 'ปกติ';
                    heroTag.className = "text-[11px] font-bold px-2 py-0.5 rounded-full bg-emerald-50 text-emerald-700 dark:bg-emerald-950/60 dark:text-emerald-400";
                    heroDesc.textContent = 'พัดลมปิดอยู่ เลือกระดับความแรง 1–5 ด้านล่างเพื่อเริ่มเป่าลม';
                }
            }

            // Sync level buttons if not in program
            if (d.ph === 0 && !isCutOrStopped) {
                let match = null;
                for (let i = 0; i < LV_PCT.length; i++) {
                    if (LV_PCT[i] === d.b) { match = i; break; }
                }
                if (match !== null) setActiveKeypadUI(match);
            }

            // Sync Program timer
            updateProgramStatus(d.pg, d.ph);

            // Assistant recommendations (Using HI)
            evaluateAdvice(d, hi);

            // Raw debug
            $('#raw-tele-box').textContent = 
                `Ta:${d.ta.toFixed(1)}C, Rh:${d.rh}%, HI:${hi.toFixed(1)}C, Tb:${d.tb.toFixed(1)}C, SOC:${d.soc}%, EffB:${d.b}%, EffN:${d.n}%, Bat:${d.bat}%, F:0b${d.f.toString(2).padStart(3, '0')}, Pg:${d.pg}, Ph:${d.ph}`;
        }

        // --- Active Level Keypad UI ---
        function setActiveKeypadUI(lvl) {
            activeLevel = lvl;
            $$('.btn-level').forEach(btn => {
                const bl = parseInt(btn.dataset.lvl);
                if (bl === lvl) {
                    if (bl === 0) {
                        btn.className = "btn-level btn-touch-active min-h-[58px] rounded-xl border-2 border-slate-700 bg-slate-700 text-white font-bold flex flex-col items-center justify-center p-1 ring-2 ring-slate-400";
                    } else {
                        btn.className = "btn-level btn-touch-active min-h-[58px] rounded-xl border-2 border-brand-500 bg-brand-600 text-white font-bold flex flex-col items-center justify-center p-1 ring-2 ring-brand-300 dark:ring-brand-800 shadow-md";
                        btn.querySelector('.text-slate-500')?.classList.replace('text-slate-500', 'text-brand-100');
                    }
                } else {
                    btn.className = "btn-level btn-touch-active min-h-[58px] rounded-xl border-2 border-slate-200 dark:border-slate-700 bg-white dark:bg-slate-800 text-slate-800 dark:text-slate-100 font-bold flex flex-col items-center justify-center p-1 transition-all";
                }
            });
            $('#ctrl-box-status').textContent = lvl === 0 ? 'OFF (0%)' : `ระดับ ${lvl} (${LV_PCT[lvl]}%)`;
        }

        // --- Advice Engine (Uses HI, Non-Medical) ---
        function evaluateAdvice(d, hi) {
            const isCut = (d.f & 1) !== 0 && d.tb >= 55;
            const isDerate = (d.f & 4) !== 0 || d.tb >= 50;
            const isBatHot = d.tb >= 40 && d.tb < 50;
            const isLowSoc = d.soc < 20;
            const isHighHI = hi >= 36.0;

            let targetId = null, icon = '💡', msg = '';

            if (isCut) {
                targetId = 'rec_cut'; icon = '🛡️';
                msg = 'ระบบหยุดเพื่อความปลอดภัย กรุณารอให้แบตเตอรี่ระบายความร้อนต่ำลง แล้วกดปุ่มรีเซ็ต';
            } else if (isDerate) {
                targetId = 'rec_derate'; icon = '🌡️';
                msg = 'ระบบลดกำลังพัดลมให้อัตโนมัติ (60%) เพื่อช่วยลดความร้อนสะสมของแบตเตอรี่';
            } else if (userTappedStop && (d.f & 1)) {
                targetId = 'rec_stop'; icon = '🛑';
                msg = 'พัดลมหยุดทันทีแล้ว เลือกระดับความแรง 1–5 ได้ทุกเมื่อที่คุณต้องการเริ่มใช้งานต่อ';
            } else if (isBatHot) {
                targetId = 'rec_bathot'; icon = '💨';
                msg = 'พัดลมแบตเตอรี่กำลังทำงานเพื่อระบายความร้อน รักษาอุณหภูมิเซลล์ให้อยู่ในเกณฑ์ปลอดภัย';
            } else if (isLowSoc) {
                targetId = 'rec_lowsoc'; icon = '⚡';
                msg = 'แบตเตอรี่เหลือน้อยกว่า 20% แนะนำให้เตรียมสลับก้อนใหม่ หรือชาร์จไฟเมื่อเสร็จงาน';
            } else if (isHighHI) {
                targetId = 'rec_hi'; icon = '☀️';
                msg = `ดัชนีความร้อนสูง (${hi.toFixed(1)}°C) สภาพแวดล้อมร้อนจัด ควรเลือกพักในที่ร่มสักครู่และจิบน้ำอย่างสม่ำเสมอ`;
            }

            const box = $('#advice-box');
            if (targetId && !dismissedAdvice.has(targetId)) {
                currentAdviceId = targetId;
                $('#advice-icon').textContent = icon;
                $('#advice-text').textContent = msg;
                box.classList.remove('hidden');
            } else if (!targetId && dismissedAdvice.has('onboard')) {
                box.classList.add('hidden');
            }
        }

        function dismissAdvice() {
            dismissedAdvice.add(currentAdviceId);
            $('#advice-box').classList.add('hidden');
            showToast('ซ่อนคำแนะนำนี้แล้ว');
        }

        // --- Program Countdown Logic ---
        function updateProgramStatus(pg, ph) {
            currentProg = pg;
            const isRunning = (ph > 0);
            const titleEl = $('#prog-active-title');
            const timerEl = $('#prog-countdown-timer');

            $('#btn-prog-0').classList.toggle('border-brand-500', isRunning && pg === 0);
            $('#btn-prog-1').classList.toggle('border-brand-500', isRunning && pg === 1);
            $('#btn-prog-2').classList.toggle('border-brand-500', isRunning && pg === 2);

            if (!isRunning) {
                titleEl.textContent = 'ไม่ได้รันโปรแกรม';
                timerEl.textContent = '--:--';
                currentPhase = 0;
                if (countdownTimer) { clearInterval(countdownTimer); countdownTimer = null; }
                return;
            }

            titleEl.textContent = `▶ ${PROG_NAMES[pg]} (เฟส ${ph})`;

            if (currentPhase !== ph) {
                currentPhase = ph;
                phaseStartTime = Date.now();

                let durMin = 0;
                if (pg === 0 || pg === 1) durMin = PROG_CONFIGS[pg][ph - 1] ? PROG_CONFIGS[pg][ph - 1][1] : 0;
                else durMin = localCustom[ph - 1] ? localCustom[ph - 1].min : 0;

                const isFast = $('#chk-fast-test')?.checked || false;
                phaseDurationSec = isFast ? (durMin * 6) : (durMin * 60);

                if (countdownTimer) clearInterval(countdownTimer);
                if (durMin > 0) {
                    countdownTimer = setInterval(() => {
                        const elapsed = Math.floor((Date.now() - phaseStartTime) / 1000);
                        const remain = Math.max(0, phaseDurationSec - elapsed);
                        const m = Math.floor(remain / 60);
                        const s = remain % 60;
                        timerEl.textContent = `${m.toString().padStart(2, '0')}:${s.toString().padStart(2, '0')}`;
                    }, 500);
                } else {
                    timerEl.textContent = 'ต่อเนื่อง';
                }
            }
        }

        function startProgram(progId) {
            if (isAutoActive) {
                setAutoMaster(false);
                showToast('ปิดระบบ Auto เพื่อเริ่มโปรแกรม');
            }
            sendCmd(7, progId);
            showToast(`เริ่มโปรแกรม ${PROG_NAMES[progId]}`);
        }

        // --- Web Bluetooth Implementation ---
        async function connectBLE() {
            haptic(30);
            try {
                if (!navigator.bluetooth) {
                    showToast('⚠️ เบราว์เซอร์นี้ไม่รองรับ Web Bluetooth (โปรดเปิดผ่าน HTTPS หรือแอป PWA)');
                    return;
                }

                bleDevice = await navigator.bluetooth.requestDevice({
                    filters: [{ namePrefix: 'SmartVest' }],
                    optionalServices: [SVC_UUID]
                });

                bleDevice.addEventListener('gattserverdisconnected', onBLEDisconnected);

                const server = await bleDevice.gatt.connect();
                const service = await server.getPrimaryService(SVC_UUID);
                cmdChar = await service.getCharacteristic(CMD_UUID);
                telChar = await service.getCharacteristic(TEL_UUID);

                await telChar.startNotifications();
                telChar.addEventListener('characteristicvaluechanged', e => {
                    const view = e.target.value;
                    if (view.byteLength >= 12) updateTelemetryUI(parseTelemetry(view));
                });

                setConnectionUI('BLE', bleDevice.name);
            } catch (err) {
                console.warn('BLE connect canceled:', err);
                setConnectionUI('NONE');
            }
        }

        function onBLEDisconnected() {
            setConnectionUI('NONE');
            $('#disconn-alert').classList.remove('hidden');
            $('#hero-title').textContent = '⚠️ ขาดการเชื่อมต่อ Bluetooth';
            $('#hero-desc').textContent = 'พัดลมอาจยังคงทำงานอยู่ที่ค่าล่าสุด กรุณาแตะเชื่อมต่อใหม่เพื่อสั่งการ';
            showToast('⚠️ ขาดการเชื่อมต่อ Bluetooth!', 4000);

            if (isAutoActive) {
                pauseAutoMode('ขาดการเชื่อมต่อ Bluetooth');
            }
        }

        function setConnectionUI(mode, name = '') {
            connMode = mode;
            const btn = $('#btn-ble-connect');
            const txt = $('#ble-status-text');
            const dot = $('#ble-ping-dot');
            const alertBox = $('#disconn-alert');

            if (mode === 'BLE') {
                txt.textContent = name || 'SmartVest';
                btn.className = "flex items-center space-x-1.5 px-3 py-1.5 rounded-full bg-emerald-50 dark:bg-emerald-950/40 border border-emerald-300 dark:border-emerald-800 text-emerald-700 dark:text-emerald-400 text-xs font-semibold";
                dot.innerHTML = `<span class="animate-ping absolute inline-flex h-full w-full rounded-full bg-emerald-400 opacity-75"></span><span class="relative inline-flex rounded-full h-2 w-2 bg-emerald-500"></span>`;
                alertBox.classList.add('hidden');
                showToast('🔵 เชื่อมต่อ Bluetooth เรียบร้อย');
            } else if (mode === 'WIFI') {
                txt.textContent = 'Wi-Fi AP';
                btn.className = "flex items-center space-x-1.5 px-3 py-1.5 rounded-full bg-sky-50 dark:bg-sky-950/40 border border-sky-300 dark:border-sky-800 text-sky-700 dark:text-sky-400 text-xs font-semibold";
                dot.innerHTML = `<span class="relative inline-flex rounded-full h-2 w-2 bg-sky-500"></span>`;
                alertBox.classList.add('hidden');
                showToast('📶 เชื่อมต่อโหมด Wi-Fi AP');
            } else if (mode === 'DEMO') {
                txt.textContent = 'Demo Mode';
                btn.className = "flex items-center space-x-1.5 px-3 py-1.5 rounded-full bg-indigo-50 dark:bg-indigo-950/40 border border-indigo-300 dark:border-indigo-800 text-indigo-700 dark:text-indigo-400 text-xs font-semibold";
                dot.innerHTML = `<span class="relative inline-flex rounded-full h-2 w-2 bg-indigo-500"></span>`;
                alertBox.classList.add('hidden');
                showToast('🧪 เปิดโหมดจำลองในเบราว์เซอร์');
            } else {
                txt.textContent = 'เชื่อมต่อ BLE';
                btn.className = "flex items-center space-x-1.5 px-3 py-1.5 rounded-full bg-slate-100 dark:bg-slate-800 hover:bg-slate-200 dark:hover:bg-slate-700 border border-slate-300 dark:border-slate-700 text-slate-700 dark:text-slate-300 text-xs font-semibold";
                dot.innerHTML = `<span class="relative inline-flex rounded-full h-2 w-2 bg-slate-400"></span>`;
            }
        }

        // --- Keypad Click Handlers ---
        $$('.btn-level').forEach(btn => {
            btn.onclick = () => {
                const lvl = parseInt(btn.dataset.lvl);
                if (isAutoActive) {
                    setAutoMaster(false);
                    showToast('สลับเป็นระดับเอง');
                }
                setActiveKeypadUI(lvl);
                sendCmd(3, lvl);
                $('#slider-b').value = LV_PCT[lvl];
                $('#disp-slider-b').textContent = LV_PCT[lvl] + '%';
                const neckVal = Math.min(LV_PCT[lvl], 70);
                $('#slider-n').value = neckVal;
                $('#disp-slider-n').textContent = neckVal + '%';
            };
        });

        function handleEmergencyStop() {
            haptic(50);
            if (isAutoActive) {
                pauseAutoMode('ผู้ใช้กด STOP');
            }
            setActiveKeypadUI(0);
            sendCmd(4, 0);
            showToast('⛔ สั่งหยุดพัดลมทันที');
        }

        function handleSliderB(v) {
            $('#disp-slider-b').textContent = v + '%';
            sendCmd(1, parseInt(v));
        }

        function handleSliderN(v) {
            const cap = Math.min(parseInt(v), 70); // STRICT CAP <= 70%
            $('#disp-slider-n').textContent = cap + '%';
            sendCmd(2, cap);
        }

        // --- Control Box Tabs Switcher ---
        function switchControlTab(tab) {
            ['lvl', 'auto', 'prog'].forEach(t => {
                const isTarget = (t === tab);
                $(`#ctrl-tab-${t}`).className = isTarget 
                    ? "px-3 py-1.5 rounded-xl text-xs font-bold bg-brand-50 text-brand-600 dark:bg-brand-950/60 dark:text-brand-400 transition-all"
                    : "px-3 py-1.5 rounded-xl text-xs font-semibold text-slate-500 hover:text-slate-800 dark:hover:text-slate-200 transition-all";
                $(`#pane-ctrl-${t}`).classList.toggle('hidden', !isTarget);
            });
        }

        // --- SECTION F: AUTO SYSTEM & PRESETS ---
        const PRESETS_STORAGE_KEY = 'smartvest.presets.v1';

        const BUILTIN_PRESETS = [
            {
                id: 'builtin_outdoor',
                name: 'กลางแจ้ง',
                icon: '☀️',
                isBuiltin: true,
                maxLevel: 5,
                bands: [
                    { minHI: -99, maxHI: 26.9, level: 0 },
                    { minHI: 27.0, maxHI: 31.9, level: 2 },
                    { minHI: 32.0, maxHI: 35.9, level: 3 },
                    { minHI: 36.0, maxHI: 39.9, level: 4 },
                    { minHI: 40.0, maxHI: 999, level: 5 }
                ]
            },
            {
                id: 'builtin_indoor',
                name: 'ในร่ม',
                icon: '🏢',
                isBuiltin: true,
                maxLevel: 3,
                bands: [
                    { minHI: -99, maxHI: 27.9, level: 0 },
                    { minHI: 28.0, maxHI: 29.9, level: 1 },
                    { minHI: 30.0, maxHI: 31.9, level: 2 },
                    { minHI: 32.0, maxHI: 999, level: 3 }
                ]
            }
        ];

        let userPresets = [];
        let activePresetId = 'builtin_outdoor';
        let isAutoActive = false;
        let isAutoPaused = false;
        let autoPauseReason = '';
        let lastAutoAppliedLevel = null;
        let lastAutoLevelChangeTime = 0;
        let autoEvalInterval = null;

        function loadPresets() {
            try {
                const stored = localStorage.getItem(PRESETS_STORAGE_KEY);
                userPresets = stored ? JSON.parse(stored) : [];
            } catch (e) {
                userPresets = [];
            }
            renderPresetsList();
        }

        function saveUserPresetsToStorage() {
            localStorage.setItem(PRESETS_STORAGE_KEY, JSON.stringify(userPresets));
            renderPresetsList();
        }

        function getAllPresets() {
            return [...BUILTIN_PRESETS, ...userPresets];
        }

        function renderPresetsList() {
            const list = $('#presets-list');
            list.innerHTML = getAllPresets().map(p => {
                const isSelected = (p.id === activePresetId);
                return `
                    <div class="flex items-center justify-between p-2 rounded-xl border ${isSelected ? 'border-brand-500 bg-brand-50/50 dark:bg-brand-950/40' : 'border-slate-200 dark:border-slate-700 bg-white dark:bg-slate-800'} text-xs transition-all">
                        <div class="flex items-center gap-2 cursor-pointer flex-1" onclick="selectPreset('${p.id}')">
                            <span class="text-base">${p.icon}</span>
                            <div>
                                <span class="font-bold text-slate-900 dark:text-white">${p.name}</span>
                                ${p.isBuiltin ? '<span class="text-[9px] px-1 py-0.2 bg-slate-100 dark:bg-slate-700 rounded text-slate-500 ml-1">พรีเซ็ตหลัก</span>' : ''}
                                <div class="text-[10px] text-slate-400">เพดาน: ระดับ ${p.maxLevel} • ${p.bands.length} ช่วง</div>
                            </div>
                        </div>
                        <div class="flex items-center gap-1">
                            <button onclick="clonePreset('${p.id}')" class="p-1 text-slate-400 hover:text-brand-600" title="คัดลอก"><i data-lucide="copy" class="w-3.5 h-3.5"></i></button>
                            ${!p.isBuiltin ? `
                                <button onclick="editUserPreset('${p.id}')" class="p-1 text-slate-400 hover:text-amber-500" title="แก้ไข"><i data-lucide="edit-2" class="w-3.5 h-3.5"></i></button>
                                <button onclick="confirmDeletePreset('${p.id}')" class="p-1 text-slate-400 hover:text-rose-500" title="ลบ"><i data-lucide="trash-2" class="w-3.5 h-3.5"></i></button>
                            ` : ''}
                        </div>
                    </div>
                `;
            }).join('');
            lucide.createIcons();
        }

        function selectPreset(id) {
            activePresetId = id;
            renderPresetsList();
            const p = getAllPresets().find(x => x.id === id);
            showToast(`เลือกพรีเซ็ต: ${p ? p.name : ''}`);
            if (isAutoActive) {
                lastAutoAppliedLevel = null;
                evaluateAutoLogic();
            }
        }

        function toggleAutoMaster(enabled) {
            setAutoMaster(enabled);
            if (enabled) {
                if (currentProg > 0) {
                    sendCmd(8, 0); // Stop program
                    showToast('หยุดโปรแกรมเพื่อเปิดระบบ Auto');
                }
                showToast('เปิดระบบระบายความร้อนอัตโนมัติ');
            } else {
                showToast('ปิดระบบอัตโนมัติ');
            }
        }

        function setAutoMaster(enabled) {
            isAutoActive = enabled;
            $('#auto-master-switch').checked = enabled;
            $('#hero-active-mode-badge').textContent = enabled ? 'โหมด: อัตโนมัติ (Auto)' : 'โหมด: ควบคุมเอง';

            if (enabled) {
                isAutoPaused = false;
                $('#auto-paused-card').classList.add('hidden');
                $('#auto-status-desc').textContent = 'กำลังทำงาน • ประเมินทุก 2 วินาที';
                lastAutoAppliedLevel = null;
                lastAutoLevelChangeTime = 0;
                if (!autoEvalInterval) autoEvalInterval = setInterval(evaluateAutoLogic, 2000);
                evaluateAutoLogic();
            } else {
                if (autoEvalInterval) { clearInterval(autoEvalInterval); autoEvalInterval = null; }
                $('#auto-status-desc').textContent = 'ปิดการทำงาน';
                $('#auto-paused-card').classList.add('hidden');
            }
        }

        function pauseAutoMode(reason) {
            if (!isAutoActive) return;
            isAutoPaused = true;
            autoPauseReason = reason;
            $('#auto-status-desc').textContent = `หยุดชั่วคราว: ${reason}`;
            $('#auto-pause-reason').textContent = reason;
            $('#auto-paused-card').classList.remove('hidden');
        }

        function resumeAutoMode() {
            if (!isAutoActive) {
                setAutoMaster(true);
                return;
            }
            isAutoPaused = false;
            $('#auto-paused-card').classList.add('hidden');
            $('#auto-status-desc').textContent = 'กำลังทำงาน • ประเมินทุก 2 วินาที';
            lastAutoAppliedLevel = null;
            showToast('กลับสู่โหมดอัตโนมัติแล้ว');
            evaluateAutoLogic();
        }

        function evaluateAutoLogic() {
            if (!isAutoActive || isAutoPaused) return;

            // Check auto pause triggers
            if (connMode === 'BLE' && !bleDevice?.gatt?.connected) {
                pauseAutoMode('ขาดการเชื่อมต่อ Bluetooth');
                return;
            }
            if ((latestTel.f & 1) !== 0) {
                pauseAutoMode(latestTel.tb >= 55 ? 'ระบบตัดวงจร (ความร้อนเกิน)' : 'ผู้ใช้กด STOP');
                return;
            }
            if ((latestTel.f & 4) !== 0 || latestTel.tb >= 50) {
                pauseAutoMode('แบตร้อน ≥ 50°C (ระบบลดโหลด)');
                return;
            }

            const hi = calculateHeatIndex(latestTel.ta, latestTel.rh);
            const preset = getAllPresets().find(p => p.id === activePresetId) || BUILTIN_PRESETS[0];

            // Evaluate bands
            let targetLvl = 0;
            for (const band of preset.bands) {
                if (hi >= band.minHI && hi <= band.maxHI) {
                    targetLvl = band.level;
                    break;
                }
            }

            // Cap at maxLevel
            targetLvl = Math.min(targetLvl, preset.maxLevel);

            // If level needs to change, check dwell time (30s) and hysteresis (1C)
            if (lastAutoAppliedLevel !== targetLvl) {
                const now = Date.now();
                if (lastAutoAppliedLevel !== null && (now - lastAutoLevelChangeTime < 30000)) {
                    // Holding dwell time
                    return;
                }

                // Apply level
                lastAutoAppliedLevel = targetLvl;
                lastAutoLevelChangeTime = now;
                sendCmd(3, targetLvl);
                setActiveKeypadUI(targetLvl);
                showToast(`Auto: ปรับเป็นระดับ ${targetLvl} (HI: ${hi.toFixed(1)}°C)`);
            }
        }

        // --- Custom Preset Editing / Creation ---
        let editingPresetId = null;

        function openCreatePresetModal() {
            if (userPresets.length >= 10) {
                showToast('จำกัดพรีเซ็ตที่สร้างเองได้ไม่เกิน 10 รายการ');
                return;
            }
            editingPresetId = null;
            $('#preset-modal-title').textContent = 'สร้างพรีเซ็ตใหม่';
            $('#inp-preset-name').value = '';
            $('#sel-preset-icon').value = '⚡';
            $('#sel-preset-maxlvl').value = '5';
            renderPresetBandRows([
                { minHI: 0, maxHI: 28, level: 0 },
                { minHI: 28.1, maxHI: 34, level: 2 },
                { minHI: 34.1, maxHI: 60, level: 4 }
            ]);
            togglePresetModal(true);
        }

        function clonePreset(id) {
            const p = getAllPresets().find(x => x.id === id);
            if (!p) return;
            if (userPresets.length >= 10) {
                showToast('จำกัดพรีเซ็ตที่สร้างเองได้ไม่เกิน 10 รายการ');
                return;
            }
            editingPresetId = null;
            $('#preset-modal-title').textContent = `คัดลอกจาก ${p.name}`;
            $('#inp-preset-name').value = `${p.name} (คัดลอก)`;
            $('#sel-preset-icon').value = p.icon;
            $('#sel-preset-maxlvl').value = String(p.maxLevel);
            renderPresetBandRows(p.bands);
            togglePresetModal(true);
        }

        function editUserPreset(id) {
            const p = userPresets.find(x => x.id === id);
            if (!p) return;
            editingPresetId = id;
            $('#preset-modal-title').textContent = `แก้ไขพรีเซ็ต: ${p.name}`;
            $('#inp-preset-name').value = p.name;
            $('#sel-preset-icon').value = p.icon;
            $('#sel-preset-maxlvl').value = String(p.maxLevel);
            renderPresetBandRows(p.bands);
            togglePresetModal(true);
        }

        function togglePresetModal(show) {
            $('#preset-modal').classList.toggle('hidden', !show);
        }

        function renderPresetBandRows(bands) {
            const container = $('#preset-bands-container');
            container.innerHTML = bands.map((b, idx) => `
                <div class="band-row flex items-center gap-1.5 p-1.5 rounded-lg bg-slate-50 dark:bg-slate-800 border border-slate-200 dark:border-slate-700">
                    <span class="text-[10px] font-bold text-slate-400 w-4">${idx + 1}</span>
                    <input type="number" step="0.1" value="${b.minHI <= -90 ? 0 : b.minHI}" class="band-min w-16 p-1 border rounded bg-white dark:bg-slate-900 text-center text-xs">
                    <span class="text-slate-400">-</span>
                    <input type="number" step="0.1" value="${b.maxHI >= 900 ? 55 : b.maxHI}" class="band-max w-16 p-1 border rounded bg-white dark:bg-slate-900 text-center text-xs">
                    <span class="text-[10px] text-slate-400">°C →</span>
                    <select class="band-lvl flex-1 p-1 border rounded bg-white dark:bg-slate-900 text-xs">
                        <option value="0" ${b.level === 0 ? 'selected' : ''}>OFF</option>
                        <option value="1" ${b.level === 1 ? 'selected' : ''}>ระดับ 1</option>
                        <option value="2" ${b.level === 2 ? 'selected' : ''}>ระดับ 2</option>
                        <option value="3" ${b.level === 3 ? 'selected' : ''}>ระดับ 3</option>
                        <option value="4" ${b.level === 4 ? 'selected' : ''}>ระดับ 4</option>
                        <option value="5" ${b.level === 5 ? 'selected' : ''}>ระดับ 5</option>
                    </select>
                    <button type="button" onclick="this.parentElement.remove()" class="text-rose-500 font-bold px-1">✕</button>
                </div>
            `).join('');
        }

        function addPresetBandRow() {
            const rows = $$('.band-row');
            if (rows.length >= 5) {
                showToast('กำหนดช่วง HI ได้สูงสุด 5 ช่วง');
                return;
            }
            const container = $('#preset-bands-container');
            const div = document.createElement('div');
            div.className = "band-row flex items-center gap-1.5 p-1.5 rounded-lg bg-slate-50 dark:bg-slate-800 border border-slate-200 dark:border-slate-700";
            div.innerHTML = `
                <span class="text-[10px] font-bold text-slate-400 w-4">${rows.length + 1}</span>
                <input type="number" step="0.1" value="35" class="band-min w-16 p-1 border rounded bg-white dark:bg-slate-900 text-center text-xs">
                <span class="text-slate-400">-</span>
                <input type="number" step="0.1" value="40" class="band-max w-16 p-1 border rounded bg-white dark:bg-slate-900 text-center text-xs">
                <span class="text-[10px] text-slate-400">°C →</span>
                <select class="band-lvl flex-1 p-1 border rounded bg-white dark:bg-slate-900 text-xs">
                    <option value="0">OFF</option>
                    <option value="1">ระดับ 1</option>
                    <option value="2">ระดับ 2</option>
                    <option value="3">ระดับ 3</option>
                    <option value="4">ระดับ 4</option>
                    <option value="5" selected>ระดับ 5</option>
                </select>
                <button type="button" onclick="this.parentElement.remove()" class="text-rose-500 font-bold px-1">✕</button>
            `;
            container.appendChild(div);
        }

        function savePresetForm() {
            const name = $('#inp-preset-name').value.trim();
            if (!name) {
                showToast('กรุณากรอกชื่อพรีเซ็ต');
                return;
            }

            const rows = $$('.band-row');
            if (rows.length === 0) {
                showToast('ต้องมีอย่างน้อย 1 ช่วง HI');
                return;
            }

            const bands = [];
            for (const r of rows) {
                const min = parseFloat(r.querySelector('.band-min').value);
                const max = parseFloat(r.querySelector('.band-max').value);
                const lvl = parseInt(r.querySelector('.band-lvl').value);

                if (isNaN(min) || isNaN(max) || min >= max) {
                    showToast('ค่า HI เริ่มต้นต้องน้อยกว่าค่าสิ้นสุด');
                    return;
                }
                bands.push({ minHI: min, maxHI: max, level: lvl });
            }

            // Sort bands by minHI ascending
            bands.sort((a, b) => a.minHI - b.minHI);

            // Validate no overlaps
            for (let i = 0; i < bands.length - 1; i++) {
                if (bands[i].maxHI > bands[i + 1].minHI) {
                    showToast(`ช่วงอุณหภูมิซ้อนทับกัน (${bands[i].maxHI}°C > ${bands[i + 1].minHI}°C)`);
                    return;
                }
            }

            const icon = $('#sel-preset-icon').value;
            const maxLevel = parseInt($('#sel-preset-maxlvl').value);

            if (editingPresetId) {
                const target = userPresets.find(x => x.id === editingPresetId);
                if (target) {
                    target.name = name;
                    target.icon = icon;
                    target.maxLevel = maxLevel;
                    target.bands = bands;
                }
            } else {
                userPresets.push({
                    id: 'custom_' + Date.now(),
                    name,
                    icon,
                    isBuiltin: false,
                    maxLevel,
                    bands
                });
            }

            saveUserPresetsToStorage();
            togglePresetModal(false);
            showToast('บันทึกพรีเซ็ตเรียบร้อย');
        }

        function confirmDeletePreset(id) {
            const p = userPresets.find(x => x.id === id);
            if (!p) return;
            showConfirm(
                'ลบพรีเซ็ต',
                `คุณต้องการลบพรีเซ็ต "${p.name}" หรือไม่? ข้อมูลจะถูกลบถาวร`,
                () => {
                    userPresets = userPresets.filter(x => x.id !== id);
                    if (activePresetId === id) activePresetId = 'builtin_outdoor';
                    saveUserPresetsToStorage();
                    showToast('ลบพรีเซ็ตเรียบร้อย');
                }
            );
        }

        // Custom Program Table Render
        function renderCustomTable() {
            const tbody = $('#custom-table-body');
            tbody.innerHTML = [0, 1, 2, 3].map(i => `
                <tr class="border-b border-slate-100 dark:border-slate-800">
                    <td class="py-1 font-bold">เฟส ${i + 1}</td>
                    <td>
                        <select id="sel-cust-lvl-${i}" class="bg-white dark:bg-slate-800 border border-slate-200 dark:border-slate-700 rounded p-1 font-semibold">
                            <option value="255">— จบ</option>
                            <option value="0">OFF</option>
                            <option value="1">1 (20%)</option>
                            <option value="2">2 (40%)</option>
                            <option value="3">3 (60%)</option>
                            <option value="4">4 (80%)</option>
                            <option value="5">5 (100%)</option>
                        </select>
                    </td>
                    <td>
                        <input id="inp-cust-min-${i}" type="number" min="0" max="255" value="0" class="w-14 text-center bg-white dark:bg-slate-800 border border-slate-200 dark:border-slate-700 rounded p-1 font-semibold">
                    </td>
                </tr>
            `).join('');

            localCustom.forEach((item, i) => {
                $(`#sel-cust-lvl-${i}`).value = item.lvl;
                $(`#inp-cust-min-${i}`).value = item.min;
            });
        }

        function saveCustomProgram() {
            haptic(35);
            for (let i = 0; i < 4; i++) {
                const l = parseInt($(`#sel-cust-lvl-${i}`).value);
                const m = parseInt($(`#inp-cust-min-${i}`).value) || 0;
                localCustom[i] = { lvl: l, min: m };
                sendCmd(9, i, l, m);
            }
            showToast('💾 บันทึกตาราง Custom ลงบอร์ดสำเร็จ');
        }

        // Modals Toggle
        function toggleEquipmentModal(show) { $('#equipment-modal').classList.toggle('hidden', !show); }
        function toggleHelpModal(show) { $('#help-modal').classList.toggle('hidden', !show); }
        function toggleDevModal(show) { $('#dev-modal').classList.toggle('hidden', !show); }

        // --- Theme Switcher ---
        function applyTheme(theme) {
            currentTheme = theme;
            const html = document.documentElement;
            const icon = $('#theme-icon');
            if (theme === 'dark') {
                html.classList.remove('light');
                html.classList.add('dark');
                if (icon) icon.setAttribute('data-lucide', 'sun');
                if (scene) scene.background = new THREE.Color(0x0b111a);
            } else {
                html.classList.remove('dark');
                html.classList.add('light');
                if (icon) icon.setAttribute('data-lucide', 'moon');
                if (scene) scene.background = new THREE.Color(0xf8fafc);
            }
            lucide.createIcons();
            localStorage.setItem('theme', theme);
            if (telemetryChart) updateChartTheme(theme);
        }

        function toggleTheme() { applyTheme(currentTheme === 'light' ? 'dark' : 'light'); }

        // --- WebGL Capability Detection ---
        function checkWebGLSupport() {
            try {
                const canvas = document.createElement('canvas');
                return !!(window.WebGLRenderingContext && (canvas.getContext('webgl') || canvas.getContext('experimental-webgl')));
            } catch (e) {
                return false;
            }
        }

        // --- SECTION A: 3D REAL FANS SCENE (THREE.JS) ---
        let scene, camera, renderer;
        let animatedBodyFans = [], animatedNeckFans = [], animatedBatFans = [];
        let bodyAirParticles, neckAirParticles;
        let isDragging = false, previousMousePosition = { x: 0, y: 0 };
        let liveSpeedBody = 0, liveSpeedNeck = 0, liveSpeedBat = 0;
        let fansRootGroup = null;

        function init3D() {
            if (!checkWebGLSupport()) {
                $('#fallback-2d').classList.remove('hidden');
                return;
            }

            const container = $('#canvas-container');
            const canvas = $('#three-canvas');

            scene = new THREE.Scene();
            scene.background = new THREE.Color(currentTheme === 'dark' ? 0x0b111a : 0xf8fafc);

            camera = new THREE.PerspectiveCamera(45, container.clientWidth / container.clientHeight, 0.1, 1000);
            camera.position.set(0, 2.2, 7.8);
            camera.lookAt(0, 0, 0);

            renderer = new THREE.WebGLRenderer({ canvas: canvas, antialias: true, alpha: true });
            renderer.setSize(container.clientWidth, container.clientHeight);
            renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));

            const ambientLight = new THREE.AmbientLight(0xffffff, 0.85);
            scene.add(ambientLight);

            const dirLight = new THREE.DirectionalLight(0xffffff, 0.65);
            dirLight.position.set(6, 12, 8);
            scene.add(dirLight);

            createRealFanAssemblies();
            createAirParticleStreams();

            // Touch and Mouse Controls for Orbit
            function handleStart(x, y) { isDragging = true; previousMousePosition = { x, y }; }
            function handleMove(x, y) {
                if (!isDragging || !fansRootGroup) return;
                const dx = x - previousMousePosition.x;
                const dy = y - previousMousePosition.y;
                fansRootGroup.rotation.y += dx * 0.008;
                fansRootGroup.rotation.x += dy * 0.008;
                fansRootGroup.rotation.x = Math.max(-0.5, Math.min(0.5, fansRootGroup.rotation.x));
                previousMousePosition = { x, y };
            }

            container.addEventListener('mousedown', e => handleStart(e.clientX, e.clientY));
            container.addEventListener('mousemove', e => handleMove(e.clientX, e.clientY));
            window.addEventListener('mouseup', () => isDragging = false);

            container.addEventListener('touchstart', e => {
                if (e.touches.length === 1) handleStart(e.touches[0].clientX, e.touches[0].clientY);
            }, { passive: true });
            container.addEventListener('touchmove', e => {
                if (e.touches.length === 1) handleMove(e.touches[0].clientX, e.touches[0].clientY);
            }, { passive: true });
            window.addEventListener('touchend', () => isDragging = false);

            animate3D();
        }

        // Fan Assembly Generator (Realistic Casing, Hub, Blades)
        function createFanUnit(radius, depth, bladeCount, bladeColor, frameColor) {
            const group = new THREE.Group();

            const frameMat = new THREE.MeshStandardMaterial({ color: frameColor, roughness: 0.5, metalness: 0.2 });
            const hubMat = new THREE.MeshStandardMaterial({ color: 0x0284c7, roughness: 0.3, metalness: 0.6 });
            const bladeMat = new THREE.MeshPhysicalMaterial({ color: bladeColor, roughness: 0.2, metalness: 0.1, transparent: true, opacity: 0.85 });

            // Circular Housing Ring
            const ringGeo = new THREE.CylinderGeometry(radius, radius, depth, 32, 1, true);
            const ring = new THREE.Mesh(ringGeo, frameMat);
            ring.rotation.x = Math.PI / 2;
            group.add(ring);

            // Exhaust Duct Flange
            const ductGeo = new THREE.BoxGeometry(radius * 1.9, radius * 1.9, depth * 0.6);
            const duct = new THREE.Mesh(ductGeo, frameMat);
            duct.position.z = -depth * 0.2;
            group.add(duct);

            // Spinning Impeller Rotor
            const rotor = new THREE.Group();
            const hub = new THREE.Mesh(new THREE.CylinderGeometry(radius * 0.3, radius * 0.3, depth * 1.1, 16), hubMat);
            hub.rotation.x = Math.PI / 2;
            rotor.add(hub);

            // Aerodynamic Curved Blades
            for (let i = 0; i < bladeCount; i++) {
                const angle = (Math.PI * 2 / bladeCount) * i;
                const bladeGeo = new THREE.BoxGeometry(radius * 0.6, depth * 0.15, radius * 0.2);
                const blade = new THREE.Mesh(bladeGeo, bladeMat);
                blade.position.set(Math.cos(angle) * (radius * 0.45), Math.sin(angle) * (radius * 0.45), 0);
                blade.rotation.z = angle + 0.4;
                blade.rotation.y = 0.35;
                rotor.add(blade);
            }

            group.add(rotor);
            return { group, rotor };
        }

        function createRealFanAssemblies() {
            fansRootGroup = new THREE.Group();

            // 1. Dual Body Fans (70mm, 7 blades) - Channel A & B
            const bodyFan1 = createFanUnit(1.0, 0.4, 7, 0x38bdf8, 0x1e293b);
            bodyFan1.group.position.set(-1.4, -0.4, 0);
            fansRootGroup.add(bodyFan1.group);
            animatedBodyFans.push(bodyFan1.rotor);

            const bodyFan2 = createFanUnit(1.0, 0.4, 7, 0x38bdf8, 0x1e293b);
            bodyFan2.group.position.set(1.4, -0.4, 0);
            fansRootGroup.add(bodyFan2.group);
            animatedBodyFans.push(bodyFan2.rotor);

            // 2. Dual Shoulder Fans (40mm, 5 blades) - Neck
            const neckFan1 = createFanUnit(0.65, 0.3, 5, 0x7dd3fc, 0x334155);
            neckFan1.group.position.set(-1.1, 1.4, -0.2);
            neckFan1.group.rotation.x = -0.3;
            fansRootGroup.add(neckFan1.group);
            animatedNeckFans.push(neckFan1.rotor);

            const neckFan2 = createFanUnit(0.65, 0.3, 5, 0x7dd3fc, 0x334155);
            neckFan2.group.position.set(1.1, 1.4, -0.2);
            neckFan2.group.rotation.x = -0.3;
            fansRootGroup.add(neckFan2.group);
            animatedNeckFans.push(neckFan2.rotor);

            // 3. Battery Micro Fan (30mm, 5 blades) with Heatsink Block
            const batFan = createFanUnit(0.5, 0.25, 5, 0xf59e0b, 0x475569);
            batFan.group.position.set(0, -0.1, -0.3);
            
            // Heatsink base plate behind battery fan
            const heatSink = new THREE.Mesh(
                new THREE.BoxGeometry(1.2, 1.2, 0.25),
                new THREE.MeshStandardMaterial({ color: 0x64748b, roughness: 0.3, metalness: 0.8 })
            );
            heatSink.position.set(0, -0.1, -0.5);
            fansRootGroup.add(heatSink);

            fansRootGroup.add(batFan.group);
            animatedBatFans.push(batFan.rotor);

            // Base mounting chassis
            const baseChassis = new THREE.Mesh(
                new THREE.CylinderGeometry(2.4, 2.6, 0.2, 32),
                new THREE.MeshStandardMaterial({ color: 0x0f172a, roughness: 0.6 })
            );
            baseChassis.position.y = -1.5;
            fansRootGroup.add(baseChassis);

            scene.add(fansRootGroup);
        }

        function createAirParticleStreams() {
            // Body Fan Particles
            const countBody = 100;
            const geoBody = new THREE.BufferGeometry();
            const posBody = new Float32Array(countBody * 3);
            for (let i = 0; i < countBody; i++) {
                const side = (i % 2 === 0) ? -1.4 : 1.4;
                posBody[i * 3] = side + (Math.random() - 0.5) * 0.8;
                posBody[i * 3 + 1] = -0.4 + (Math.random() - 0.5) * 0.8;
                posBody[i * 3 + 2] = 0.2 + Math.random() * 2.5;
            }
            geoBody.setAttribute('position', new THREE.BufferAttribute(posBody, 3));
            bodyAirParticles = new THREE.Points(geoBody, new THREE.PointsMaterial({ color: 0x38bdf8, size: 0.08, transparent: true, opacity: 0.7 }));
            fansRootGroup.add(bodyAirParticles);

            // Neck Fan Particles
            const countNeck = 60;
            const geoNeck = new THREE.BufferGeometry();
            const posNeck = new Float32Array(countNeck * 3);
            for (let i = 0; i < countNeck; i++) {
                const side = (i % 2 === 0) ? -1.1 : 1.1;
                posNeck[i * 3] = side + (Math.random() - 0.5) * 0.5;
                posNeck[i * 3 + 1] = 1.4 + Math.random() * 1.5;
                posNeck[i * 3 + 2] = -0.2 + (Math.random() - 0.5) * 0.6;
            }
            geoNeck.setAttribute('position', new THREE.BufferAttribute(posNeck, 3));
            neckAirParticles = new THREE.Points(geoNeck, new THREE.PointsMaterial({ color: 0x7dd3fc, size: 0.07, transparent: true, opacity: 0.7 }));
            fansRootGroup.add(neckAirParticles);
        }

        function animate3D() {
            requestAnimationFrame(animate3D);

            // Gentle Idle Rotation if not dragging
            if (!isDragging && fansRootGroup) fansRootGroup.rotation.y += 0.002;

            // 1. Body Fans Rotation (b %)
            const speedFactorBody = (liveSpeedBody / 100) * 0.45;
            animatedBodyFans.forEach(f => f.rotation.z += speedFactorBody);

            // 2. Neck Fans Rotation (n %)
            const speedFactorNeck = (liveSpeedNeck / 100) * 0.45;
            animatedNeckFans.forEach(f => f.rotation.z += speedFactorNeck);

            // 3. Battery Fan Rotation (bat %)
            const speedFactorBat = (liveSpeedBat / 100) * 0.45;
            animatedBatFans.forEach(f => f.rotation.z += speedFactorBat);

            // Particles Flow
            if (bodyAirParticles) {
                if (liveSpeedBody > 0) {
                    const pos = bodyAirParticles.geometry.attributes.position.array;
                    for (let i = 0; i < pos.length / 3; i++) {
                        pos[i * 3 + 2] += 0.02 + (speedFactorBody * 0.08);
                        if (pos[i * 3 + 2] > 3.0) pos[i * 3 + 2] = 0.3;
                    }
                    bodyAirParticles.geometry.attributes.position.needsUpdate = true;
                    bodyAirParticles.visible = true;
                } else {
                    bodyAirParticles.visible = false;
                }
            }

            if (neckAirParticles) {
                if (liveSpeedNeck > 0) {
                    const pos = neckAirParticles.geometry.attributes.position.array;
                    for (let i = 0; i < pos.length / 3; i++) {
                        pos[i * 3 + 1] += 0.02 + (speedFactorNeck * 0.08);
                        if (pos[i * 3 + 1] > 3.0) pos[i * 3 + 1] = 1.4;
                    }
                    neckAirParticles.geometry.attributes.position.needsUpdate = true;
                    neckAirParticles.visible = true;
                } else {
                    neckAirParticles.visible = false;
                }
            }

            renderer.render(scene, camera);
        }

        function resetCamera() {
            if (fansRootGroup) fansRootGroup.rotation.set(0, 0, 0);
            camera.position.set(0, 2.2, 7.8);
            camera.lookAt(0, 0, 0);
        }

        // --- Chart.js Telemetry Graph ---
        let telemetryChart = null;
        function initChart() {
            const ctx = document.getElementById('telemetryChart').getContext('2d');
            telemetryChart = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: Array.from({length: 12}, (_, i) => `${(12 - i) * 2}s`),
                    datasets: [
                        {
                            label: 'อากาศ (°C)',
                            data: Array(12).fill(34),
                            borderColor: '#0ea5e9',
                            backgroundColor: 'rgba(14, 165, 233, 0.1)',
                            borderWidth: 2,
                            tension: 0.35,
                            fill: true
                        },
                        {
                            label: 'แบต NTC (°C)',
                            data: Array(12).fill(30),
                            borderColor: '#f59e0b',
                            backgroundColor: 'rgba(245, 158, 11, 0.1)',
                            borderWidth: 2,
                            tension: 0.35,
                            fill: true
                        }
                    ]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: { legend: { display: false } },
                    scales: {
                        x: { ticks: { color: currentTheme === 'dark' ? '#94a3b8' : '#64748b', font: { size: 10 } } },
                        y: { ticks: { color: currentTheme === 'dark' ? '#94a3b8' : '#64748b', font: { size: 10 } } }
                    }
                }
            });

            // Update chart every 2 seconds
            setInterval(() => {
                if (!telemetryChart) return;
                const d0 = telemetryChart.data.datasets[0].data;
                const d1 = telemetryChart.data.datasets[1].data;
                d0.shift(); d0.push(latestTel.ta);
                d1.shift(); d1.push(latestTel.tb);
                telemetryChart.update('none');
            }, 2000);
        }

        function updateChartTheme(theme) {
            if (!telemetryChart) return;
            const col = (theme === 'dark') ? '#94a3b8' : '#64748b';
            telemetryChart.options.scales.x.ticks.color = col;
            telemetryChart.options.scales.y.ticks.color = col;
            telemetryChart.update();
        }

        // --- Wi-Fi & Demo Simulation Loops ---
        function startWifiMode() {
            if (pollTimer) clearInterval(pollTimer);
            setConnectionUI('WIFI');
            const poll = () => {
                fetch('/api/state').then(r => r.json()).then(d => updateTelemetryUI(d)).catch(() => {});
            };
            poll();
            pollTimer = setInterval(poll, 500);
        }

        const demo = {
            ta: 34.0, rh: 60, tb: 30.5, soc: 85,
            sb: 0, sn: 0, hot: false, stopped: false, cut: false, batFan: 0,
            progRunning: false, progId: 0, phaseId: 0, phaseStart: 0, fast: false
        };

        function startDemoMode() {
            setConnectionUI('DEMO');
            if (pollTimer) clearInterval(pollTimer);
            pollTimer = setInterval(demoTick, 500);
        }

        function demoHandleCmd(c, v, a, b) {
            if (demo.cut && ((c >= 1 && c <= 3) || c === 7)) return;
            if (c >= 1 && c <= 5) demo.progRunning = false;
            switch (c) {
                case 1: demo.sb = v; demo.stopped = false; break;
                case 2: demo.sn = Math.min(v, 70); demo.stopped = false; break;
                case 3:
                    demo.sb = LV_PCT[v]; demo.sn = Math.min(LV_PCT[v], 70);
                    demo.stopped = false; break;
                case 4: demo.stopped = true; demo.sb = demo.sn = 0; break;
                case 5: demo.stopped = demo.hot = demo.cut = false; demo.sb = demo.sn = 0; break;
                case 6: demo.hot = !demo.hot; break;
                case 7:
                    demo.progId = v; demo.progRunning = true; demo.phaseId = 1; demo.phaseStart = Date.now();
                    applyDemoProgLevel(v, 0); break;
                case 8: demo.progRunning = false; break;
                case 9: if (v >= 0 && v < 4) localCustom[v] = { lvl: a, min: b }; break;
                case 12: demo.fast = (v !== 0); break;
            }
        }

        function applyDemoProgLevel(prog, idx) {
            let l = (prog < 2) ? (PROG_CONFIGS[prog][idx] ? PROG_CONFIGS[prog][idx][0] : 0) : (localCustom[idx] ? localCustom[idx].lvl : 0);
            if (l <= 5) {
                demo.sb = LV_PCT[l]; demo.sn = Math.min(LV_PCT[l], 70);
            }
        }

        function demoTick() {
            if (demo.progRunning) {
                if (demo.cut || demo.stopped) demo.progRunning = false;
                else {
                    let durMin = (demo.progId < 2) ? PROG_CONFIGS[demo.progId][demo.phaseId - 1][1] : localCustom[demo.phaseId - 1].min;
                    const durMs = durMin * (demo.fast ? 6000 : 60000);
                    if (durMs > 0 && Date.now() - demo.phaseStart >= durMs) {
                        const next = demo.phaseId + 1;
                        let ok = (demo.progId < 2) ? (next <= PROG_CONFIGS[demo.progId].length && PROG_CONFIGS[demo.progId][next - 1][0] <= 5) : (next <= 4 && localCustom[next - 1].lvl <= 5);
                        if (ok) {
                            demo.phaseId = next; demo.phaseStart = Date.now();
                            applyDemoProgLevel(demo.progId, next - 1);
                        } else {
                            demo.progRunning = false;
                        }
                    }
                }
            }

            demo.ta = Math.min(45, Math.max(20, demo.ta + (Math.random() - 0.5) * 0.2));
            let effB = demo.sb, effN = demo.sn;
            if (demo.tb >= 50) { effB = Math.round(effB * 0.6); effN = Math.round(effN * 0.6); }
            if (demo.tb >= 60) demo.cut = true;
            if (demo.cut || demo.stopped) effB = effN = 0;

            const targetTb = 28 + 0.4 * (demo.ta - 25) + 0.06 * (effB + effN) + (demo.hot ? 45 : 0) - 0.06 * demo.batFan;
            demo.tb += (targetTb - demo.tb) * 0.1;

            if (demo.cut) demo.batFan = 0;
            else if (demo.tb >= 40 || (demo.batFan > 0 && demo.tb >= 37)) demo.batFan = Math.min(100, Math.max(15, Math.round(30 + (demo.tb - 40) * 5)));
            else demo.batFan = 0;

            demo.soc = Math.max(0, demo.soc - 0.002 * (1 + (effB + effN) / 100));
            const flags = (demo.cut || demo.stopped ? 1 : 0) | (demo.hot ? 2 : 0) | (demo.tb >= 50 ? 4 : 0);

            updateTelemetryUI({
                ta: demo.ta, rh: demo.rh, tb: demo.tb, soc: Math.round(demo.soc),
                b: effB, n: effN, bat: demo.batFan, f: flags,
                ph: demo.progRunning ? demo.phaseId : 0, pg: demo.progId
            });
        }

        // Apply Simulation Badges (Requirement C)
        function applySimulationBadges() {
            document.querySelectorAll('.sim-badge').forEach(el => {
                el.style.display = SIMULATED_SENSORS ? 'inline-block' : 'none';
            });
            const note = $('#sim-sensors-note');
            if (note) note.style.display = SIMULATED_SENSORS ? 'block' : 'none';
        }

        // --- Initialization ---
        window.addEventListener('DOMContentLoaded', () => {
            init3D();
            initChart();
            renderCustomTable();
            loadPresets();
            applyTheme(currentTheme);
            applySimulationBadges();

            // PWA Service Worker Registration
            if ('serviceWorker' in navigator && location.protocol === 'https:') {
                navigator.serviceWorker.register('sw.js').catch(() => {});
            }
        });
    </script>
</body>
</html>

)HTMLPAGE";
