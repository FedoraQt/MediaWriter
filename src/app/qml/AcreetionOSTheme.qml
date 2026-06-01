/*
 * AcreetionOS Media Writer
 * Copyright (C) 2026 Natalie Spiva <natalie@acreetionos.org>
 *
 * AcreetionOS brand theme — matches acreetionos.org styling
 *
 * Primary:   #1793d1 (brand blue)
 * Accent:    #0d6efd (darker blue for hover/active)
 * Surface:   #f8f9fa (light grey background)
 * Text:      #212529 (dark grey, not pure black)
 */

import QtQuick

QtObject {
    // Brand colours
    readonly property color brandBlue:       "#1793d1"
    readonly property color brandDark:       "#0d6efd"
    readonly property color brandLight:      "#e3f2fd"
    readonly property color accent:          "#1793d1"
    readonly property color accentHover:     "#1483c9"
    readonly property color accentPressed:   "#0f72b0"

    // Surfaces
    readonly property color surface:         "#f8f9fa"
    readonly property color surfaceAlt:      "#ffffff"
    readonly property color surfaceDark:     "#e9ecef"

    // Text
    readonly property color textPrimary:     "#212529"
    readonly property color textSecondary:   "#6c757d"
    readonly property color textOnAccent:    "#ffffff"

    // Status
    readonly property color success:         "#28a745"
    readonly property color warning:         "#ffc107"
    readonly property color error:           "#dc3545"

    // Button styling helpers
    function buttonBackground(enabled, hovered, pressed) {
        if (!enabled) return surfaceDark;
        if (pressed) return accentPressed;
        if (hovered) return accentHover;
        return accent;
    }
}
