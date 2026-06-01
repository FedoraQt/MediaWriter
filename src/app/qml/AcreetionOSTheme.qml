/*
 * AcreetionOS Media Writer
 * Copyright (C) 2026 Natalie Spiva <natalie@acreetionos.org>
 *
 * AcreetionOS brand theme — matches acreetionos.org styling
 * Supports light and dark mode, auto-detects system preference.
 *
 * Primary:   #1793d1 (brand blue)
 * Accent:    #0d6efd (darker blue for hover/active)
 */

import QtQuick
import QtQuick.Controls

QtObject {
    // Detect dark mode from the system palette or Qt quick controls style
    readonly property bool isDark: {
        // Try Qt 6.5+ colorScheme API first
        if (typeof Qt.application !== "undefined"
            && typeof Qt.application.styleHints !== "undefined"
            && typeof Qt.application.styleHints.colorScheme !== "undefined") {
            return Qt.application.styleHints.colorScheme === Qt.Dark;
        }
        // Fallback: check palette luminance
        var p = palette;
        if (p && p.window) {
            var c = p.window;
            var lum = 0.299 * c.r + 0.587 * c.g + 0.114 * c.b;
            return lum < 0.5;
        }
        return false;
    }

    // Brand colours — same in both modes
    readonly property color brandBlue:       "#1793d1"
    readonly property color brandBlueHover:  "#1483c9"
    readonly property color brandBluePressed:"#0f72b0"

    // Surfaces — switch between light and dark
    readonly property color surface:         isDark ? "#1e1e1e" : "#f8f9fa"
    readonly property color surfaceAlt:      isDark ? "#2d2d2d" : "#ffffff"
    readonly property color surfaceDark:     isDark ? "#3d3d3d" : "#e9ecef"
    readonly property color surfaceCard:     isDark ? "#252526" : "#ffffff"

    // Text — switch between light and dark
    readonly property color textPrimary:     isDark ? "#e0e0e0" : "#212529"
    readonly property color textSecondary:   isDark ? "#a0a0a0" : "#6c757d"
    readonly property color textOnAccent:    "#ffffff"

    // Status
    readonly property color success:         isDark ? "#2ea043" : "#28a745"
    readonly property color warning:         isDark ? "#d29922" : "#ffc107"
    readonly property color error:           isDark ? "#f85149" : "#dc3545"

    // Borders
    readonly property color borderLight:     isDark ? "#3d3d3d" : "#dee2e6"
    readonly property color borderFocus:     brandBlue

    // Button backgrounds with state
    function buttonBackground(enabled, hovered, pressed) {
        if (!enabled) return surfaceDark;
        if (pressed) return brandBluePressed;
        if (hovered) return brandBlueHover;
        return brandBlue;
    }

    // Radio button indicator colours
    readonly property color indicatorBorder: isDark ? "#888888" : "#6c757d"
    readonly property color indicatorChecked: brandBlue
}
