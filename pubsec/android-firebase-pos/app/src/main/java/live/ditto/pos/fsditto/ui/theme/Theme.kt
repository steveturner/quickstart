package live.ditto.pos.fsditto.ui.theme

import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable

private val PosColorScheme = darkColorScheme(
    primary = AccentGreen,
    onPrimary = DarkBackground,
    primaryContainer = AccentGreenContainer,
    onPrimaryContainer = OnAccentGreenContainer,
    secondary = AccentOrange,
    onSecondary = DarkBackground,
    secondaryContainer = AccentOrangeContainer,
    onSecondaryContainer = OnAccentOrangeContainer,
    tertiary = AccentGreenDim,
    tertiaryContainer = AccentGreenContainer,
    onTertiaryContainer = OnAccentGreenContainer,
    background = DarkBackground,
    onBackground = DarkOnSurface,
    surface = DarkSurface,
    onSurface = DarkOnSurface,
    surfaceVariant = DarkSurfaceVariant,
    onSurfaceVariant = DarkOnSurfaceVariant,
    error = ErrorRed,
    errorContainer = ErrorContainer,
    onErrorContainer = OnErrorContainer,
    outline = NeutralChip
)

@Composable
fun PosTheme(content: @Composable () -> Unit) {
    MaterialTheme(
        colorScheme = PosColorScheme,
        typography = Typography,
        content = content
    )
}
