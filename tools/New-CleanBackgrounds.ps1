param(
    [string]$RepositoryRoot = (Split-Path -Parent $PSScriptRoot)
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

Add-Type -AssemblyName System.Drawing

function New-RoundedPath {
    param(
        [System.Drawing.RectangleF]$Rectangle,
        [float]$Radius
    )

    $diameter = $Radius * 2
    $path = [System.Drawing.Drawing2D.GraphicsPath]::new()
    $path.AddArc($Rectangle.X, $Rectangle.Y, $diameter, $diameter, 180, 90)
    $path.AddArc($Rectangle.Right - $diameter, $Rectangle.Y, $diameter, $diameter, 270, 90)
    $path.AddArc($Rectangle.Right - $diameter, $Rectangle.Bottom - $diameter, $diameter, $diameter, 0, 90)
    $path.AddArc($Rectangle.X, $Rectangle.Bottom - $diameter, $diameter, $diameter, 90, 90)
    $path.CloseFigure()
    return $path
}

function Fill-RoundedRectangle {
    param(
        [System.Drawing.Graphics]$Graphics,
        [System.Drawing.Brush]$Brush,
        [System.Drawing.RectangleF]$Rectangle,
        [float]$Radius
    )

    $path = New-RoundedPath -Rectangle $Rectangle -Radius $Radius
    try { $Graphics.FillPath($Brush, $path) } finally { $path.Dispose() }
}

function Draw-RoundedRectangle {
    param(
        [System.Drawing.Graphics]$Graphics,
        [System.Drawing.Pen]$Pen,
        [System.Drawing.RectangleF]$Rectangle,
        [float]$Radius
    )

    $path = New-RoundedPath -Rectangle $Rectangle -Radius $Radius
    try { $Graphics.DrawPath($Pen, $path) } finally { $path.Dispose() }
}

function Draw-CoverImage {
    param(
        [System.Drawing.Graphics]$Graphics,
        [System.Drawing.Image]$Image,
        [System.Drawing.RectangleF]$Target
    )

    $sourceRatio = $Image.Width / $Image.Height
    $targetRatio = $Target.Width / $Target.Height
    if ($sourceRatio -gt $targetRatio) {
        $sourceHeight = $Image.Height
        $sourceWidth = $sourceHeight * $targetRatio
        $sourceX = ($Image.Width - $sourceWidth) / 2
        $sourceY = 0
    } else {
        $sourceWidth = $Image.Width
        $sourceHeight = $sourceWidth / $targetRatio
        $sourceX = 0
        $sourceY = ($Image.Height - $sourceHeight) / 2
    }

    $source = [System.Drawing.RectangleF]::new(
        [float]$sourceX,
        [float]$sourceY,
        [float]$sourceWidth,
        [float]$sourceHeight
    )
    $Graphics.DrawImage($Image, $Target, $source, [System.Drawing.GraphicsUnit]::Pixel)
}

function New-CleanBackground {
    param(
        [string]$PhotoPath,
        [string]$OutputPath,
        [System.Drawing.Color]$Accent,
        [string]$OverlayPath
    )

    $bitmap = [System.Drawing.Bitmap]::new(800, 480, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $photo = [System.Drawing.Image]::FromFile($PhotoPath)

    try {
        $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
        $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
        $graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
        $graphics.Clear([System.Drawing.Color]::FromArgb(255, 1, 4, 8))

        $panelBrush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(255, 8, 19, 25))
        $rightBrush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(255, 3, 9, 13))
        $borderPen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(255, 29, 48, 57), 1)
        try {
            Fill-RoundedRectangle $graphics $panelBrush ([System.Drawing.RectangleF]::new(4, 4, 792, 74)) 10
            Fill-RoundedRectangle $graphics $panelBrush ([System.Drawing.RectangleF]::new(2, 82, 127, 346)) 8
            Fill-RoundedRectangle $graphics $rightBrush ([System.Drawing.RectangleF]::new(617, 82, 179, 346)) 8
            Fill-RoundedRectangle $graphics $panelBrush ([System.Drawing.RectangleF]::new(4, 430, 792, 46)) 8
            Draw-RoundedRectangle $graphics $borderPen ([System.Drawing.RectangleF]::new(4, 4, 792, 74)) 10
            Draw-RoundedRectangle $graphics $borderPen ([System.Drawing.RectangleF]::new(2, 82, 127, 346)) 8
            Draw-RoundedRectangle $graphics $borderPen ([System.Drawing.RectangleF]::new(617, 82, 179, 346)) 8
            Draw-RoundedRectangle $graphics $borderPen ([System.Drawing.RectangleF]::new(4, 430, 792, 46)) 8
        } finally {
            $panelBrush.Dispose()
            $rightBrush.Dispose()
            $borderPen.Dispose()
        }

        # The photo is decoration only. Interactive cards, labels and icons are
        # deliberately absent so the web editor/firmware can render them once.
        $headerPath = New-RoundedPath ([System.Drawing.RectangleF]::new(4, 4, 792, 74)) 10
        $oldClip = $graphics.Clip
        try {
            $graphics.SetClip($headerPath)
            Draw-CoverImage $graphics $photo ([System.Drawing.RectangleF]::new(326, 4, 470, 74))

            $fade = [System.Drawing.Drawing2D.LinearGradientBrush]::new(
                [System.Drawing.PointF]::new(290, 40),
                [System.Drawing.PointF]::new(650, 40),
                [System.Drawing.Color]::FromArgb(255, 8, 19, 25),
                [System.Drawing.Color]::FromArgb(18, 8, 19, 25)
            )
            $shade = [System.Drawing.Drawing2D.LinearGradientBrush]::new(
                [System.Drawing.PointF]::new(500, 4),
                [System.Drawing.PointF]::new(500, 78),
                [System.Drawing.Color]::FromArgb(15, 1, 4, 8),
                [System.Drawing.Color]::FromArgb(118, 1, 4, 8)
            )
            try {
                $graphics.FillRectangle($fade, 250, 4, 546, 74)
                $graphics.FillRectangle($shade, 326, 4, 470, 74)
            } finally {
                $fade.Dispose()
                $shade.Dispose()
            }

            if ($OverlayPath) {
                $overlay = [System.Drawing.Image]::FromFile($OverlayPath)
                try {
                    # A slightly oversized transparent product cutout keeps the
                    # actual machine recognisable inside the shallow header.
                    $graphics.DrawImage($overlay, [System.Drawing.RectangleF]::new(646, -17, 112, 112))
                } finally {
                    $overlay.Dispose()
                }
            }
        } finally {
            $graphics.Clip = $oldClip
            $oldClip.Dispose()
            $headerPath.Dispose()
        }

        $accentBrush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(215, $Accent.R, $Accent.G, $Accent.B))
        $glowBrush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(28, $Accent.R, $Accent.G, $Accent.B))
        try {
            $graphics.FillRectangle($accentBrush, 4, 76, 792, 2)
            $graphics.FillRectangle($glowBrush, 129, 82, 488, 346)
        } finally {
            $accentBrush.Dispose()
            $glowBrush.Dispose()
        }

        $outputDirectory = Split-Path -Parent $OutputPath
        [System.IO.Directory]::CreateDirectory($outputDirectory) | Out-Null
        $bitmap.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Png)
    } finally {
        $photo.Dispose()
        $graphics.Dispose()
        $bitmap.Dispose()
    }
}

function Resize-CleanMaster {
    param(
        [string]$MasterPath,
        [string]$OutputPath
    )

    $source = [System.Drawing.Image]::FromFile($MasterPath)
    $bitmap = [System.Drawing.Bitmap]::new(800, 480, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    try {
        $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
        $graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
        $graphics.DrawImage($source, 0, 0, 800, 480)
        $bitmap.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Png)
    } finally {
        $graphics.Dispose()
        $bitmap.Dispose()
        $source.Dispose()
    }
}

$assetRoot = Join-Path $RepositoryRoot "firmware/MacroDeckUI/assets"
$sourceRoot = Join-Path $assetRoot "source_photos"

$backgrounds = @(
    @{
        Photo = Join-Path $sourceRoot "prusa_mk4_official.jpg"
        Overlay = Join-Path $sourceRoot "prusa_mk4_product_official_512.png"
        Output = Join-Path $assetRoot "ui_prusa_clean_800x480.png"
        Accent = [System.Drawing.ColorTranslator]::FromHtml("#F26B38")
    }
)

foreach ($background in $backgrounds) {
    if (-not (Test-Path -LiteralPath $background.Photo)) {
        throw "Missing source photo: $($background.Photo)"
    }
    New-CleanBackground -PhotoPath $background.Photo -OutputPath $background.Output -Accent $background.Accent -OverlayPath $background.Overlay
    Write-Host "Created $($background.Output)"
}

$cleanMasters = @(
    @{
        Master = Join-Path $sourceRoot "orca_original_clean_master.png"
        Output = Join-Path $assetRoot "ui_orca_clean_800x480.png"
    },
    @{
        Master = Join-Path $sourceRoot "fusion_original_clean_master.png"
        Output = Join-Path $assetRoot "ui_fusion_clean_800x480.png"
    },
    @{
        Master = Join-Path $sourceRoot "onshape_clean_master.png"
        Output = Join-Path $assetRoot "ui_onshape_clean_800x480.png"
    },
    @{
        Master = Join-Path $sourceRoot "blender_clean_master.png"
        Output = Join-Path $assetRoot "ui_blender_clean_800x480.png"
    }
)

foreach ($cleanMaster in $cleanMasters) {
    if (-not (Test-Path -LiteralPath $cleanMaster.Master)) {
        throw "Missing clean edit master: $($cleanMaster.Master)"
    }
    Resize-CleanMaster -MasterPath $cleanMaster.Master -OutputPath $cleanMaster.Output
    Write-Host "Created $($cleanMaster.Output)"
}
