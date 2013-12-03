﻿/*
 * Carla Host API
 * Copyright (C) 2011-2013 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the doc/GPL.txt file.
 */

#ifndef CARLA_HOST_H_INCLUDED
#define CARLA_HOST_H_INCLUDED

#include "CarlaBackend.h"

#ifdef __cplusplus
using CarlaBackend::BinaryType;
using CarlaBackend::PluginType;
using CarlaBackend::PluginCategory;
using CarlaBackend::InternalParameterIndex;
using CarlaBackend::EngineCallbackOpcode;
using CarlaBackend::EngineOption;
using CarlaBackend::EngineProcessMode;
using CarlaBackend::EngineTransportMode;
using CarlaBackend::EngineCallbackFunc;
using CarlaBackend::ParameterData;
using CarlaBackend::ParameterRanges;
using CarlaBackend::MidiProgramData;
using CarlaBackend::CustomData;
using CarlaBackend::EngineDriverDeviceInfo;
using CarlaBackend::CarlaEngine;
using CarlaBackend::CarlaPlugin;
#endif

/*!
 * @defgroup CarlaHostAPI Carla Host API
 *
 * The Carla Host API.
 *
 * This API makes it possible to use the Carla Backend in a standalone host application..
 *
 * None of the returned values in this API calls need to be deleted or free'd.\n
 * When a function fails (returns false or NULL), use carla_get_last_error() to find out what went wrong.
 * @{
 */

/*!
 * File callback opcodes.\n
 * Front-ends must always block-wait for user input.
 * @see FileCallbackFunc and carla_set_file_callback()
 */
typedef enum {
    /*!
     * Debug.\n
     * This opcode is undefined and used only for testing purposes.
     */
    FILE_CALLBACK_DEBUG = 0,

    /*!
     * Open file or folder.
     */
    FILE_CALLBACK_OPEN = 1,

    /*!
     * Save file or folder.
     */
    FILE_CALLBACK_SAVE = 2

} FileCallbackOpcode;

/*!
 * File callback function.
 * @see FileCallbackOpcode
 */
typedef const char* (*FileCallbackFunc)(void* ptr, FileCallbackOpcode action, bool isDir, const char* title, const char* filter);

/*!
 * Information about a loaded plugin.
 * @see carla_get_plugin_info()
 */
typedef struct _CarlaPluginInfo {
    /*!
     * Plugin type.
     */
    PluginType type;

    /*!
     * Plugin category.
     */
    PluginCategory category;

    /*!
     * Plugin hints.
     * @see PluginHints
     */
    unsigned int hints;

    /*!
     * Plugin options available for the user to change.
     * @see PluginOptions
     */
    unsigned int optionsAvailable;

    /*!
     * Plugin options currently enabled.\n
     * Some options are enabled but not available, which means they will always be on.
     * @see PluginOptions
     */
    unsigned int optionsEnabled;

    /*!
     * Plugin filename.\n
     * This can be the plugin binary or resource file.
     */
    const char* filename;

    /*!
     * Plugin name.\n
     * This name is unique within a Carla instance.
     * @see carla_get_real_plugin_name()
     */
    const char* name;

    /*!
     * Plugin label or URI.
     */
    const char* label;

    /*!
     * Plugin author/maker.
     */
    const char* maker;

    /*!
     * Plugin copyright/license.
     */
    const char* copyright;

    /*!
     * Icon name for this plugin, in lowercase.\n
     * Default is "plugin".
     */
    const char* iconName;

    /*!
     * Patchbay client Id for this plugin.\n
     * When 0, Id is considered invalid or unused.
     */
    int patchbayClientId;

    /*!
     * Plugin unique Id.\n
     * This Id is dependant on the plugin type and may sometimes be 0.
     */
    long uniqueId;

#ifdef __cplusplus
    /*!
     * C++ constructor.
     */
    _CarlaPluginInfo()
        : type(CarlaBackend::PLUGIN_NONE),
          category(CarlaBackend::PLUGIN_CATEGORY_NONE),
          hints(0x0),
          optionsAvailable(0x0),
          optionsEnabled(0x0),
          filename(nullptr),
          name(nullptr),
          label(nullptr),
          maker(nullptr),
          copyright(nullptr),
          iconName(nullptr),
          patchbayClientId(0),
          uniqueId(0) {}

    /*!
     * C++ destructor.
     */
    ~_CarlaPluginInfo()
    {
        if (label != nullptr)
        {
            delete[] label;
            label = nullptr;
        }
        if (maker != nullptr)
        {
            delete[] maker;
            maker = nullptr;
        }
        if (copyright != nullptr)
        {
            delete[] copyright;
            copyright = nullptr;
        }
    }
#endif
} CarlaPluginInfo;

/*!
 * Information about an internal Carla plugin.
 * @see carla_get_internal_plugin_info()
 */
typedef struct _CarlaNativePluginInfo {
    /*!
     * Plugin category.
     */
    PluginCategory category;

    /*!
     * Plugin hints.
     * @see PluginHints
     */
    unsigned int hints;

    /*!
     * Number of audio inputs.
     */
    uint32_t audioIns;

    /*!
     * Number of audio outputs.
     */
    uint32_t audioOuts;

    /*!
     * Number of MIDI inputs.
     */
    uint32_t midiIns;

    /*!
     * Number of MIDI outputs.
     */
    uint32_t midiOuts;

    /*!
     * Number of input parameters.
     */
    uint32_t parameterIns;

    /*!
     * Number of output parameters.
     */
    uint32_t parameterOuts;

    /*!
     * Plugin name.
     */
    const char* name;

    /*!
     * Plugin label.
     */
    const char* label;

    /*!
     * Plugin author/maker.
     */
    const char* maker;

    /*!
     * Plugin copyright/license.
     */
    const char* copyright;

#ifdef __cplusplus
    /*!
     * C++ constructor.
     */
    _CarlaNativePluginInfo()
        : category(CarlaBackend::PLUGIN_CATEGORY_NONE),
          hints(0x0),
          audioIns(0),
          audioOuts(0),
          midiIns(0),
          midiOuts(0),
          parameterIns(0),
          parameterOuts(0),
          name(nullptr),
          label(nullptr),
          maker(nullptr),
          copyright(nullptr) {}
#endif
} CarlaNativePluginInfo;

/*!
 * Port count information, used for Audio and MIDI ports and parameters.
 * @see carla_get_audio_port_count_info()
 * @see carla_get_midi_port_count_info()
 * @see carla_get_parameter_count_info()
 */
typedef struct _CarlaPortCountInfo {
    /*!
     * Number of inputs.
     */
    uint32_t ins;

    /*!
     * Number of outputs.
     */
    uint32_t outs;

#ifdef __cplusplus
    /*!
     * C++ constructor.
     */
    _CarlaPortCountInfo()
        : ins(0),
          outs(0) {}
#endif
} CarlaPortCountInfo;

/*!
 * Parameter information.
 * @see carla_get_parameter_info()
 */
typedef struct _CarlaParameterInfo {
    /*!
     * Parameter name.
     */
    const char* name;

    /*!
     * Parameter symbol.
     */
    const char* symbol;

    /*!
     * Parameter unit.
     */
    const char* unit;

    /*!
     * Number of scale points.
     * @see CarlaScalePointInfo
     */
    uint32_t scalePointCount;

#ifdef __cplusplus
    /*!
     * C++ constructor.
     */
    _CarlaParameterInfo()
        : name(nullptr),
          symbol(nullptr),
          unit(nullptr),
          scalePointCount(0) {}

    /*!
     * C++ destructor.
     */
    ~_CarlaParameterInfo()
    {
        if (name != nullptr)
        {
            delete[] name;
            name = nullptr;
        }
        if (symbol != nullptr)
        {
            delete[] symbol;
            symbol = nullptr;
        }
        if (unit != nullptr)
        {
            delete[] unit;
            unit = nullptr;
        }
    }
#endif
} CarlaParameterInfo;

/*!
 * Parameter scale point information.
 * @see carla_get_parameter_scalepoint_info()
 */
typedef struct _CarlaScalePointInfo {
    /*!
     * Scale point value.
     */
    float value;

    /*!
     * Scale point label.
     */
    const char* label;

#ifdef __cplusplus
    /*!
     * C++ constructor.
     */
    _CarlaScalePointInfo()
        : value(0.0f),
          label(nullptr) {}

    /*!
     * C++ destructor.
     */
    ~_CarlaScalePointInfo()
    {
        if (label != nullptr)
        {
            delete[] label;
            label = nullptr;
        }
    }
#endif
} CarlaScalePointInfo;

/*!
 * Transport information.
 * @see carla_get_transport_info()
 */
typedef struct _CarlaTransportInfo {
    /*!
     * Wherever transport is playing.
     */
    bool playing;

    /*!
     * Current transport frame.
     */
    uint64_t frame;

    /*!
     * Bar.
     */
    int32_t bar;

    /*!
     * Beat.
     */
    int32_t beat;

    /*!
     * Tick.
     */
    int32_t tick;

    /*!
     * Beats per minute.
     */
    double bpm;

#ifdef __cplusplus
    /*!
     * C++ constructor.
     */
    _CarlaTransportInfo()
        : playing(false),
          frame(0),
          bar(0),
          beat(0),
          tick(0),
          bpm(0.0) {}
#endif
} CarlaTransportInfo;

/*!
 * Get the complete license text of used third-party code and features.\n
 * Returned string is in basic html format.
 */
CARLA_EXPORT const char* carla_get_complete_license_text();

/*!
 * Get all the supported file extensions in carla_load_file().\n
 * Returned string uses this syntax:
 * @code
 * "*.ext1;*.ext2;*.ext3"
 * @endcode
 */
CARLA_EXPORT const char* carla_get_supported_file_extensions();

/*!
 * Get how many engine drivers are available.
 */
CARLA_EXPORT unsigned int carla_get_engine_driver_count();

/*!
 * Get an engine driver name.
 * @param index Driver index
 */
CARLA_EXPORT const char* carla_get_engine_driver_name(unsigned int index);

/*!
 * Get the device names of an engine driver.
 * @param index Driver index
 */
CARLA_EXPORT const char* const* carla_get_engine_driver_device_names(unsigned int index);

/*!
 * Get information about a device driver.
 * @param index Driver index
 * @param name  Device name
 */
CARLA_EXPORT const EngineDriverDeviceInfo* carla_get_engine_driver_device_info(unsigned int index, const char* name);

/*!
 * Get how many internal plugins are available.
 */
CARLA_EXPORT unsigned int carla_get_internal_plugin_count();

/*!
 * Get information about an internal plugin.
 * @param index Internal plugin Id
 */
CARLA_EXPORT const CarlaNativePluginInfo* carla_get_internal_plugin_info(unsigned int index);

#ifdef __cplusplus
/*!
 * Get the currently used Host engine.
 * @note C++ only
 */
CARLA_EXPORT CarlaEngine* carla_get_host_engine();
#endif

/*!
 * Initialize the engine.\n
 * Make sure to call carla_engine_idle() at regular intervals afterwards.
 * @param driverName Driver to use
 * @param clientName Engine master client name
 */
CARLA_EXPORT bool carla_engine_init(const char* driverName, const char* clientName);

#ifdef BUILD_BRIDGE
/*!
 * Initialize the engine in bridged mode.
 * @param audioBaseName   Shared memory key for audio pool
 * @param controlBaseName Shared memory key for control messages
 * @param clientName      Engine master client name
 */
CARLA_EXPORT bool carla_engine_init_bridge(const char audioBaseName[6], const char controlBaseName[6], const char* clientName);
#endif

/*!
 * Close the engine.\n
 * This function always closes the engine even if it returns false.\n
 * In other words, even when something goes wrong when closing the engine it still be closed nonetheless.
 */
CARLA_EXPORT bool carla_engine_close();

/*!
 * Idle the engine.\n
 * Do not call this if the engine is not running.
 */
CARLA_EXPORT void carla_engine_idle();

/*!
 * Check if the engine is running.
 */
CARLA_EXPORT bool carla_is_engine_running();

/*!
 * Tell the engine it's about to close.\n
 * This is used to prevent the engine thread(s) from reactivating.
 */
CARLA_EXPORT void carla_set_engine_about_to_close();

/*!
 * Set the engine callback function.
 * @param func Callback function
 * @param ptr  Callback pointer
 */
CARLA_EXPORT void carla_set_engine_callback(EngineCallbackFunc func, void* ptr);

#ifndef BUILD_BRIDGE
/*!
 * Set an engine option.
 * @param option   Option
 * @param value    Value as number
 * @param valueStr Value as string
 */
CARLA_EXPORT void carla_set_engine_option(EngineOption option, int value, const char* valueStr);
#endif

/*!
 * Set the file callback function.
 * @param func Callback function
 * @param ptr  Callback pointer
 */
CARLA_EXPORT void carla_set_file_callback(FileCallbackFunc func, void* ptr);

/*!
 * Load a file of any type.\n
 * This will try to load a generic file as a plugin,
 * either by direct handling (Csound, GIG, SF2 and SFZ) or by using an internal plugin (like Audio and MIDI).
 * @param Filename Filename
 * @see carla_get_supported_file_extensions()
 */
CARLA_EXPORT bool carla_load_file(const char* filename);

/*!
 * Load a Carla project file.
 * @param Filename Filename
 * @note Currently loaded plugins are not removed; call carla_remove_all_plugins() first if needed.
 */
CARLA_EXPORT bool carla_load_project(const char* filename);

/*!
 * Save current project to a file.
 * @param Filename Filename
 */
CARLA_EXPORT bool carla_save_project(const char* filename);

/*!
 * Connect patchbay ports \a portA and \a portB.
 */
CARLA_EXPORT bool carla_patchbay_connect(int portA, int portB);

/*!
 * Disconnect patchbay connection \a connectionId.
 */
CARLA_EXPORT bool carla_patchbay_disconnect(int connectionId);

/*!
 * Force the engine to resend all patchbay clients, ports and connections again.
 */
CARLA_EXPORT bool carla_patchbay_refresh();

/*!
 * Start playback of the engine transport.
 */
CARLA_EXPORT void carla_transport_play();

/*!
 * Pause the engine transport.
 */
CARLA_EXPORT void carla_transport_pause();

/*!
 * Relocate the engine transport to \a frames.
 */
CARLA_EXPORT void carla_transport_relocate(uint32_t frames);

/*!
 * Get the current transport frame.
 */
CARLA_EXPORT uint64_t carla_get_current_transport_frame();

/*!
 * Get the engine transport information.
 */
CARLA_EXPORT const CarlaTransportInfo* carla_get_transport_info();

/*!
 * Add new plugin.\n
 * If you don't know the binary type, use BINARY_NATIVE.
 */
CARLA_EXPORT bool carla_add_plugin(BinaryType btype, PluginType ptype, const char* filename, const char* name, const char* label, const void* extraPtr);

/*!
 * Remove plugin with id \a pluginId.
 */
CARLA_EXPORT bool carla_remove_plugin(unsigned int pluginId);

/*!
 * Remove all plugins.
 */
CARLA_EXPORT bool carla_remove_all_plugins();

/*!
 * Rename plugin with id \a pluginId to \a newName. \n
 * Returns the new name, or NULL if the operation failed.
 */
CARLA_EXPORT const char* carla_rename_plugin(unsigned int pluginId, const char* newName);

/*!
 * Clone plugin with id \a pluginId.
 */
CARLA_EXPORT bool carla_clone_plugin(unsigned int pluginId);

/*!
 * Prepare replace of plugin with id \a pluginId. \n
 * The next call to carla_add_plugin() will use this id, replacing the current plugin.
 * \note This function requires carla_add_plugin() to be called afterwards as soon as possible.
 */
CARLA_EXPORT bool carla_replace_plugin(unsigned int pluginId);

/*!
 * Switch plugins with id \a pluginIdA and \a pluginIdB.
 */
CARLA_EXPORT bool carla_switch_plugins(unsigned int pluginIdA, unsigned int pluginIdB);

/*!
 * Load the plugin state at \a filename.\n
 * (Plugin states have *.carxs extension).
 * @see carla_save_plugin_state()
 */
CARLA_EXPORT bool carla_load_plugin_state(unsigned int pluginId, const char* filename);

/*!
 * Load the plugin state at \a filename.\n
 * (Plugin states have *.carxs extension).
 * @see carla_load_plugin_state()
 */
CARLA_EXPORT bool carla_save_plugin_state(unsigned int pluginId, const char* filename);

/*!
 * Get a plugin's information.
 */
CARLA_EXPORT const CarlaPluginInfo* carla_get_plugin_info(unsigned int pluginId);

/*!
 * Get a plugin's audio port count information.
 */
CARLA_EXPORT const CarlaPortCountInfo* carla_get_audio_port_count_info(unsigned int pluginId);

/*!
 * Get a plugin's midi port count information.
 */
CARLA_EXPORT const CarlaPortCountInfo* carla_get_midi_port_count_info(unsigned int pluginId);

/*!
 * Get a plugin's parameter count information.
 */
CARLA_EXPORT const CarlaPortCountInfo* carla_get_parameter_count_info(unsigned int pluginId);

/*!
 * * Get a plugin's parameter information.
 */
CARLA_EXPORT const CarlaParameterInfo* carla_get_parameter_info(unsigned int pluginId, uint32_t parameterId);

/*!
 * Get a plugin's parameter scale point information.
 */
CARLA_EXPORT const CarlaScalePointInfo* carla_get_parameter_scalepoint_info(unsigned int pluginId, uint32_t parameterId, uint32_t scalePointId);

/*!
 * Get a plugin's parameter data.
 */
CARLA_EXPORT const ParameterData* carla_get_parameter_data(unsigned int pluginId, uint32_t parameterId);

/*!
 * Get a plugin's parameter ranges.
 */
CARLA_EXPORT const ParameterRanges* carla_get_parameter_ranges(unsigned int pluginId, uint32_t parameterId);

/*!
 * Get a plugin's midi program data.
 */
CARLA_EXPORT const MidiProgramData* carla_get_midi_program_data(unsigned int pluginId, uint32_t midiProgramId);

/*!
 * Get a plugin's custom data.
 */
CARLA_EXPORT const CustomData* carla_get_custom_data(unsigned int pluginId, uint32_t customDataId);

/*!
 * Get a plugin's chunk data.
 */
CARLA_EXPORT const char* carla_get_chunk_data(unsigned int pluginId);

/*!
 * Get how many parameters a plugin has.
 */
CARLA_EXPORT uint32_t carla_get_parameter_count(unsigned int pluginId);

/*!
 * Get how many programs a plugin has.
 */
CARLA_EXPORT uint32_t carla_get_program_count(unsigned int pluginId);

/*!
 * Get how many midi programs a plugin has.
 */
CARLA_EXPORT uint32_t carla_get_midi_program_count(unsigned int pluginId);

/*!
 * Get how many custom data sets a plugin has.
 * @see carla_prepare_for_save()
 */
CARLA_EXPORT uint32_t carla_get_custom_data_count(unsigned int pluginId);

/*!
 * Get a plugin's custom parameter text display.
 * @see PARAMETER_USES_CUSTOM_TEXT
 */
CARLA_EXPORT const char* carla_get_parameter_text(unsigned int pluginId, uint32_t parameterId);

/*!
 * Get a plugin's program name.
 */
CARLA_EXPORT const char* carla_get_program_name(unsigned int pluginId, uint32_t programId);

/*!
 * Get a plugin's midi program name.
 */
CARLA_EXPORT const char* carla_get_midi_program_name(unsigned int pluginId, uint32_t midiProgramId);

/*!
 * Get the plugin's real name.\n
 * This is the name the plugin uses to identify itself; may not be unique.
 */
CARLA_EXPORT const char* carla_get_real_plugin_name(unsigned int pluginId);

/*!
 * Get the current plugin's program index.
 */
CARLA_EXPORT int32_t carla_get_current_program_index(unsigned int pluginId);

/*!
 * Get the current plugin's midi program index.
 */
CARLA_EXPORT int32_t carla_get_current_midi_program_index(unsigned int pluginId);

/*!
 * Get a plugin's default parameter value.
 */
CARLA_EXPORT float carla_get_default_parameter_value(unsigned int pluginId, uint32_t parameterId);

/*!
 * Get a plugin's current parameter value.
 */
CARLA_EXPORT float carla_get_current_parameter_value(unsigned int pluginId, uint32_t parameterId);

/*!
 * Get a plugin's input peak value.\n
 * \a portId must only be either 1 or 2
 */
CARLA_EXPORT float carla_get_input_peak_value(unsigned int pluginId, unsigned short portId);

/*!
 * Get a plugin's output peak value.\n
 * \a portId must only be either 1 or 2
 */
CARLA_EXPORT float carla_get_output_peak_value(unsigned int pluginId, unsigned short portId);

/*!
 * Enable a plugin's option.
 * @see PluginOptions
 */
CARLA_EXPORT void carla_set_option(unsigned int pluginId, unsigned int option, bool yesNo);

/*!
 * Enable or disable a plugin according to \a onOff.
 */
CARLA_EXPORT void carla_set_active(unsigned int pluginId, bool onOff);

#ifndef BUILD_BRIDGE
/*!
 * Change a plugin's internal drywet value to \a value.
 */
CARLA_EXPORT void carla_set_drywet(unsigned int pluginId, float value);

/*!
 * Change a plugin's internal volume value to \a value.
 */
CARLA_EXPORT void carla_set_volume(unsigned int pluginId, float value);

/*!
 * Change a plugin's internal balance-left value to \a value.
 */
CARLA_EXPORT void carla_set_balance_left(unsigned int pluginId, float value);

/*!
 * Change a plugin's internal balance-right value to \a value.
 */
CARLA_EXPORT void carla_set_balance_right(unsigned int pluginId, float value);

/*!
 * Change a plugin's internal panning value to \a value.
 */
CARLA_EXPORT void carla_set_panning(unsigned int pluginId, float value);
#endif

/*!
 * Change a plugin's internal control channel to \a channel.
 */
CARLA_EXPORT void carla_set_ctrl_channel(unsigned int pluginId, int8_t channel);

/*!
 * Set the plugin's parameter \a parameterId to \a value.
 */
CARLA_EXPORT void carla_set_parameter_value(unsigned int pluginId, uint32_t parameterId, float value);

#ifndef BUILD_BRIDGE
/*!
 * Set the plugin's parameter \a parameterId midi channel to \a channel.
 */
CARLA_EXPORT void carla_set_parameter_midi_channel(unsigned int pluginId, uint32_t parameterId, uint8_t channel);

/*!
 * Set the plugin's parameter \a parameterId midi cc to \a cc.
 */
CARLA_EXPORT void carla_set_parameter_midi_cc(unsigned int pluginId, uint32_t parameterId, int16_t cc);
#endif

/*!
 * Change a plugin's program to \a programId.
 */
CARLA_EXPORT void carla_set_program(unsigned int pluginId, uint32_t programId);

/*!
 * Change a plugin's midi program to \a midiProgramId.
 */
CARLA_EXPORT void carla_set_midi_program(unsigned int pluginId, uint32_t midiProgramId);

/*!
 * Set a plugin's custom data set.
 */
CARLA_EXPORT void carla_set_custom_data(unsigned int pluginId, const char* type, const char* key, const char* value);

/*!
 * Set a plugin's chunk data.
 */
CARLA_EXPORT void carla_set_chunk_data(unsigned int pluginId, const char* chunkData);

/*!
 * Tell a plugin to prepare for save.\n
 * This should be called before carla_get_custom_data_count().
 */
CARLA_EXPORT void carla_prepare_for_save(unsigned int pluginId);

#ifndef BUILD_BRIDGE
/*!
 * Send a single note of a plugin.\n
 * If \a note if 0, note-off is sent; note-on otherwise.
 */
CARLA_EXPORT void carla_send_midi_note(unsigned int pluginId, uint8_t channel, uint8_t note, uint8_t velocity);
#endif

/*!
 * Tell a plugin to show its own custom UI.
 * @see PLUGIN_HAS_CUSTOM_UI
 */
CARLA_EXPORT void carla_show_custom_ui(unsigned int pluginId, bool yesNo);

/*!
 * Get the current engine buffer size.
 */
CARLA_EXPORT uint32_t carla_get_buffer_size();

/*!
 * Get the current engine sample rate.
 */
CARLA_EXPORT double carla_get_sample_rate();

/*!
 * Get the last error.
 */
CARLA_EXPORT const char* carla_get_last_error();

/*!
 * Get the current engine OSC URL (TCP).
 */
CARLA_EXPORT const char* carla_get_host_osc_url_tcp();

/*!
 * Get the current engine OSC URL (UDP).
 */
CARLA_EXPORT const char* carla_get_host_osc_url_udp();

/** @} */

#endif /* CARLA_HOST_H_INCLUDED */
