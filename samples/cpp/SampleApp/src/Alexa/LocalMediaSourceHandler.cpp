/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include "SampleApp/Alexa/LocalMediaSourceHandler.h"

// C++ Standard Library
#include <chrono>
#include <sstream>
#include <string>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace alexa {

namespace {

const std::unordered_map<aace::alexa::LocalMediaSource::Source, std::string, EnumHash> LocalMediaSourceToString = {
    { aace::alexa::LocalMediaSource::Source::BLUETOOTH, "Local BLUETOOTH" },
    { aace::alexa::LocalMediaSource::Source::USB, "Local USB" },
    { aace::alexa::LocalMediaSource::Source::FM_RADIO, "Local FM_RADIO" },
    { aace::alexa::LocalMediaSource::Source::AM_RADIO, "Local AM_RADIO" },
    { aace::alexa::LocalMediaSource::Source::SATELLITE_RADIO, "Local SATELLITE_RADIO" },
    { aace::alexa::LocalMediaSource::Source::LINE_IN, "Local LINE_IN" },
    { aace::alexa::LocalMediaSource::Source::COMPACT_DISC, "Local COMPACT_DISC" },
    { aace::alexa::LocalMediaSource::Source::SIRIUS_XM, "Local SIRIUS_XM" },
    { aace::alexa::LocalMediaSource::Source::DAB, "Local DAB" }
};

const std::unordered_map<aace::alexa::LocalMediaSource::Source, aace::alexa::LocalMediaSource::MediaType, EnumHash> LocalMediaSourceToType = {
    { aace::alexa::LocalMediaSource::Source::BLUETOOTH, aace::alexa::LocalMediaSource::MediaType::OTHER },
    { aace::alexa::LocalMediaSource::Source::USB, aace::alexa::LocalMediaSource::MediaType::OTHER },
    { aace::alexa::LocalMediaSource::Source::FM_RADIO, aace::alexa::LocalMediaSource::MediaType::STATION },
    { aace::alexa::LocalMediaSource::Source::AM_RADIO, aace::alexa::LocalMediaSource::MediaType::STATION },
    { aace::alexa::LocalMediaSource::Source::SATELLITE_RADIO, aace::alexa::LocalMediaSource::MediaType::STATION },
    { aace::alexa::LocalMediaSource::Source::LINE_IN, aace::alexa::LocalMediaSource::MediaType::OTHER },
    { aace::alexa::LocalMediaSource::Source::COMPACT_DISC, aace::alexa::LocalMediaSource::MediaType::TRACK },
    { aace::alexa::LocalMediaSource::Source::SIRIUS_XM, aace::alexa::LocalMediaSource::MediaType::STATION },
    { aace::alexa::LocalMediaSource::Source::DAB, aace::alexa::LocalMediaSource::MediaType::STATION }
};

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LocalMediaSourceHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

LocalMediaSourceHandler::LocalMediaSourceHandler(std::weak_ptr<Activity> activity,
                                                 std::weak_ptr<logger::LoggerHandler> loggerHandler,
                                                 Source source,
                                                 std::shared_ptr<aace::alexa::Speaker> speaker)
    : aace::alexa::LocalMediaSource{source, speaker}, m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)}, m_source{source} {
    auto mediaSource = LocalMediaSourceToString.find(source);
    if(mediaSource != LocalMediaSourceToString.end()) {
        m_sourceMediaProvider = mediaSource->second;
    }

    auto mediaType = LocalMediaSourceToType.find(source);
    if(mediaType != LocalMediaSourceToType.end()) {
        m_sourceMediaType = mediaType->second;
    }
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    // Expects(speaker != nullptr);
    setupUI();
}

std::weak_ptr<Activity> LocalMediaSourceHandler::getActivity() { return m_activity; }

std::weak_ptr<logger::LoggerHandler> LocalMediaSourceHandler::getLoggerHandler() { return m_loggerHandler; }

bool LocalMediaSourceHandler::play(ContentSelector contentSelectorType, const std::string &payload) {
    log(logger::LoggerHandler::Level::INFO, " play:contentSelectorType=" + std::to_string(static_cast<size_t>(contentSelectorType)) + " play:payload=" + payload);
    if (auto activity = m_activity.lock()) {
        activity->runOnUIThread([=]() {
            setFocus();
            if (auto console = m_console.lock()) {
                console->printLine(m_sourceMediaProvider, " play:", payload);
            }
        });
    }
    return true;
}

bool LocalMediaSourceHandler::playControl(PlayControlType controlType) {
    std::stringstream ss;
    ss << controlType;
    log(logger::LoggerHandler::Level::INFO, "playControl:controlType=" + ss.str());

    if (auto activity = m_activity.lock()) {
        auto text = ss.str();
        activity->runOnUIThread([=]() {
            setFocus();
            if (auto console = m_console.lock()) {
                console->printLine(m_sourceMediaProvider, " play control:", text);
            }
        });
    }
    return true;
}

bool LocalMediaSourceHandler::seek(std::chrono::milliseconds offset) {
    log(logger::LoggerHandler::Level::INFO, "seek:offset=" + std::to_string(offset.count()));
    if (auto activity = m_activity.lock()) {
        activity->runOnUIThread([=]() {
            if (auto console = m_console.lock()) {
                console->printLine(m_sourceMediaProvider, " seek:", offset.count());
            }
        });
    }
    return true;
}

bool LocalMediaSourceHandler::adjustSeek(std::chrono::milliseconds deltaOffset) {
    log(logger::LoggerHandler::Level::INFO, "adjustSeek:deltaOffset=" + std::to_string(deltaOffset.count()));
    if (auto activity = m_activity.lock()) {
        activity->runOnUIThread([=]() {
            if (auto console = m_console.lock()) {
                console->printLine(m_sourceMediaProvider, " adjust seek:", deltaOffset.count());
            }
        });
    }
    return true;
}

LocalMediaSourceHandler::LocalMediaSourceState LocalMediaSourceHandler::getState() {
    auto state = LocalMediaSourceHandler::LocalMediaSourceState{};

    std::vector<aace::alexa::LocalMediaSource::SupportedPlaybackOperation> supportedOperations{};
    std::vector<aace::alexa::LocalMediaSource::ContentSelector> supportedContentSelectors{};

    if( m_source != LocalMediaSource::Source::DAB ) { // DAB no 
        supportedOperations.insert(supportedOperations.begin(),{
            LocalMediaSource::SupportedPlaybackOperation::PLAY,
            LocalMediaSource::SupportedPlaybackOperation::PAUSE,
            LocalMediaSource::SupportedPlaybackOperation::STOP
        });
    }
    switch( m_source ) {
        case LocalMediaSource::Source::DAB :
        case LocalMediaSource::Source::SATELLITE_RADIO : 
            break;
        case LocalMediaSource::Source::AM_RADIO :
        case LocalMediaSource::Source::FM_RADIO :
            supportedContentSelectors.insert(supportedContentSelectors.begin(), {
                LocalMediaSource::ContentSelector::FREQUENCY,
                LocalMediaSource::ContentSelector::PRESET
            });
            break;
        case LocalMediaSource::Source::SIRIUS_XM :
            supportedContentSelectors.insert(supportedContentSelectors.begin(), {
                LocalMediaSource::ContentSelector::PRESET,
                LocalMediaSource::ContentSelector::CHANNEL
            });
            break;
        case LocalMediaSource::Source::BLUETOOTH :
        case LocalMediaSource::Source::USB :
        case LocalMediaSource::Source::LINE_IN :
            supportedOperations.insert(supportedOperations.end(),{
                LocalMediaSource::SupportedPlaybackOperation::FAVORITE,
                LocalMediaSource::SupportedPlaybackOperation::UNFAVORITE,
                LocalMediaSource::SupportedPlaybackOperation::NEXT,
                LocalMediaSource::SupportedPlaybackOperation::PREVIOUS,
                LocalMediaSource::SupportedPlaybackOperation::ENABLE_SHUFFLE,
                LocalMediaSource::SupportedPlaybackOperation::DISABLE_SHUFFLE,
                LocalMediaSource::SupportedPlaybackOperation::ENABLE_REPEAT_ONE,
                LocalMediaSource::SupportedPlaybackOperation::ENABLE_REPEAT,
                LocalMediaSource::SupportedPlaybackOperation::SEEK,
                LocalMediaSource::SupportedPlaybackOperation::ADJUST_SEEK,
                LocalMediaSource::SupportedPlaybackOperation::START_OVER,
                LocalMediaSource::SupportedPlaybackOperation::FAST_FORWARD,
                LocalMediaSource::SupportedPlaybackOperation::REWIND
            });
            break;
        case LocalMediaSource::Source::COMPACT_DISC :
            supportedOperations.insert(supportedOperations.end(),{
                LocalMediaSource::SupportedPlaybackOperation::NEXT,
                LocalMediaSource::SupportedPlaybackOperation::PREVIOUS,
                LocalMediaSource::SupportedPlaybackOperation::ENABLE_SHUFFLE,
                LocalMediaSource::SupportedPlaybackOperation::DISABLE_SHUFFLE,
                LocalMediaSource::SupportedPlaybackOperation::ENABLE_REPEAT_ONE,
                LocalMediaSource::SupportedPlaybackOperation::ENABLE_REPEAT,
                LocalMediaSource::SupportedPlaybackOperation::SEEK,
                LocalMediaSource::SupportedPlaybackOperation::ADJUST_SEEK,
                LocalMediaSource::SupportedPlaybackOperation::START_OVER,
                LocalMediaSource::SupportedPlaybackOperation::FAST_FORWARD,
                LocalMediaSource::SupportedPlaybackOperation::REWIND
            });
            break;
    }

    state.playbackState.supportedOperations = supportedOperations;
   
    state.sessionState.supportedContentSelectors = supportedContentSelectors;

    state.playbackState.state = m_localMediaSourceStateMap.find(m_source)->second;

    // if (auto console = m_console.lock()) {
    //     console->printLine("getState");
    // }
    return state;
}

// private

void LocalMediaSourceHandler::log(logger::LoggerHandler::Level level, const std::string &message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "LocalMediaSourceHandler", message);
}

void LocalMediaSourceHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");
}

} // namespace alexa
} // namespace sampleApp