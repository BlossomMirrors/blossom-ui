
function blend(background, foreground, amount) {
    return Qt.rgba(background.r + (foreground.r - background.r) * amount,
                   background.g + (foreground.g - background.g) * amount,
                   background.b + (foreground.b - background.b) * amount,
                   1);
}

function relativeLuminance(color) {
    const linear = c => c <= 0.03928 ? c / 12.92 : Math.pow((c + 0.055) / 1.055, 2.4);
    return 0.2126 * linear(color.r) + 0.7152 * linear(color.g) + 0.0722 * linear(color.b);
}

function contrastRatio(one, other) {
    const a = relativeLuminance(one);
    const b = relativeLuminance(other);
    return (Math.max(a, b) + 0.05) / (Math.min(a, b) + 0.05);
}

function accentOn(accent, surface, target) {
    const lightSurface = relativeLuminance(surface) > 0.18;

    let result = accent;
    let last = "";
    for (let step = 1; step <= 32 && contrastRatio(result, surface) < target; ++step) {
        const factor = 1 + 0.1 * step;
        const next = lightSurface ? Qt.darker(accent, factor) : Qt.lighter(accent, factor);
        if (next.toString() === last) {
            break;
        }
        last = next.toString();
        result = next;
    }
    return result;
}

function accentOnSelection(accent, background, alpha) {
    return accentOn(accent, blend(background, accent, alpha), 4.5);
}
