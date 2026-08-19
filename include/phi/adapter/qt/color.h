// Qt-facing layer over the canonical color contract.
//
// The conversion math itself lives in phi/adapter/v1/color.h (Qt-free) so that
// every adapter can translate between the canonical sRGB representation and its
// device's native color space without depending on Qt or this package. This
// header only pulls the canonical names into the Qt wrapper namespace and adds
// the Qt meta-type registration used by phi-core's QVariant-based channel
// values.
//
// There is exactly one Color type: phicore::adapter::Color is an alias for
// phicore::adapter::v1::Color, not a second layout-compatible struct.
#pragma once

#include <QMetaType>

#include <phi/adapter/v1/color.h>

namespace phicore::adapter {

// Canonical types.
using v1::Color;
using v1::Hsv;
using v1::LinearRgb;
using v1::Xy;

// Construction / basics.
using v1::clamp01;
using v1::colorBlack;
using v1::colorLuminance;
using v1::colorWhite;
using v1::makeColor;
using v1::wrapHue360;

// Color temperature.
using v1::kelvinToMired;
using v1::miredToKelvin;

// HSV.
using v1::colorFromHsB;
using v1::colorToHsv;
using v1::hsvToColor;

// Gamma / linear / XYZ.
using v1::colorToLinear;
using v1::linearRgbToXyz;
using v1::linearToColor;
using v1::linearToSrgb;
using v1::srgbToLinear;
using v1::xyzToLinearRgb;

// xy.
using v1::colorFromXy;
using v1::colorToXy;

} // namespace phicore::adapter

Q_DECLARE_METATYPE(phicore::adapter::Color)
