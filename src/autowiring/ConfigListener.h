#pragma once

namespace autowiring {
  /// <summary>
  /// A configuration listener interface, used to receive notifications when metadata is detected
  /// </summary>
  /// <param name="M">The metadata field to be detected</param>
  /// <remarks>
  /// </remarks>
  template<typename M>
  class ConfigBolt {
    /// <summary>
    /// Invoked when a metadata field is detected as being attached to some configuration field
    /// </summary>
    virtual void onMetadata(const aw::config_field& field, const M& data) = 0;
  };
}
