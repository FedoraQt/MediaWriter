/*
 * AcreetionOS Media Writer
 * Copyright (C) 2026 Natalie Spiva <natalie@acreetionos.org>
 *
 * Palette matches acreetionos.org CSS exactly:
 *   Body:      #121212       Box:        #222222
 *   Panel:     #1a1a1a        Border:     #333333
 *   Text:      #e5e5e5        Muted:      #b2b2b2
 *   Accent:    #2ecc71        Storm:      #61afef
 *   Flasher:   #f39c12        Purple:     #9b59b6
 *   Danger:    #e74c3c
 *
 * Based on Fedora Media Writer by the Fedora Project
 */

import QtQuick
import QtQuick.Controls

QtObject {
    // Detect dark mode — always true on AcreetionOS site
    readonly property bool isDark: true

    // AcreetionOS brand colours from acreetionos.org :root
    readonly property color green:          "#2ecc71"
    readonly property color greenHover:     "#27ae60"
    readonly property color greenPressed:   "#1e8449"
    readonly property color greenDim:       "rgba(46,204,113,0.1)"

    readonly property color storm:          "#61afef"
    readonly property color flasher:        "#f39c12"
    readonly property color purple:         "#9b59b6"
    readonly property color danger:         "#e74c3c"

    // Surfaces — dark theme matching acreetionos.org
    readonly property color surface:         "#121212"   // body-bg
    readonly property color surfaceAlt:      "#1a1a1a"   // panel-bg
    readonly property color surfaceCard:     "#222222"   // box-bg
    readonly property color surfaceDark:     "#0d0d0d"
    readonly property color surfaceHover:    "#2a2a2a"

    // Borders
    readonly property color border:          "#333333"

    // Text
    readonly property color textPrimary:     "#e5e5e5"
    readonly property color textSecondary:   "#b2b2b2"
    readonly property color textOnAccent:    "#000000"   // black on green/orange

    // Status
    readonly property color success:         "#2ecc71"
    readonly property color warning:         "#f39c12"
    readonly property color error:           "#e74c3c"

    // Accent
    readonly property color accent:          green
    readonly property color accentHover:     greenHover
    readonly property color accentPressed:   greenPressed

    // Button helper
    function buttonBackground(enabled, hovered, pressed) {
        if (!enabled) return surfaceCard;
        if (pressed) return greenPressed;
        if (hovered) return greenHover;
        return green;
    }

    // Indicator (radio / checkbox)
    readonly property color indicatorBorder:    "#555555"
    readonly property color indicatorChecked:   green
}
