#pragma once

#include "Config.h"

#include "Serialize.h"

#include <array>

#include <cstdint>

class Project;

class PlayState {
public:
    //----------------------------------------
    // Types
    //----------------------------------------

    enum ExecuteType {
        Immediate,
        Synced,
        Latched,
    };

    class TrackState {
    public:
        bool mute() const { return _state & Mute; }
        bool requestedMute() const { return _state & RequestedMute; }
        bool hasMuteRequest() const { return hasRequests(State::MuteRequests); }

        bool fill() const { return _state & Fill; }

        int pattern() const { return _pattern; }
        int requestedPattern() const { return _requestedPattern; }
        bool hasPatternRequest() const { return hasRequests(State::PatternRequests); }

        void clear();

        void write(WriteContext &context) const;
        void read(ReadContext &context);

    private:
        enum State {
            Mute                    = 1<<0,
            RequestedMute           = 1<<1,
            Fill                    = 1<<2,

            ImmediateMuteRequest    = 1<<3,
            SyncedMuteRequest       = 1<<4,
            LatchedMuteRequest      = 1<<5,

            ImmediatePatternRequest = 1<<6,
            SyncedPatternRequest    = 1<<7,
            LatchedPatternRequest   = 1<<8,

            MuteRequests = ImmediateMuteRequest | SyncedMuteRequest | LatchedMuteRequest,
            PatternRequests = ImmediatePatternRequest | SyncedPatternRequest | LatchedPatternRequest,
            ImmediateRequests = ImmediateMuteRequest | ImmediatePatternRequest,
            SyncedRequests = SyncedMuteRequest | SyncedPatternRequest,
            LatchedRequests = LatchedMuteRequest | LatchedPatternRequest
        };

        static State muteRequestFromExecuteType(ExecuteType type) {
            return State(1<<(3 + type));
        }

        static State patternRequestFromExecuteType(ExecuteType type) {
            return State(1<<(6 + type));
        }

        void setRequests(int requests) {
            _state |= uint16_t(requests);
        }

        void clearRequests(int requests) {
            _state &= ~uint16_t(requests);
        }

        bool hasRequests(int requests) const {
            return _state & uint16_t(requests);
        }

        void setMute(bool mute) {
            if (mute) {
                _state |= Mute;
            } else {
                _state &= ~Mute;
            }
        }

        void setRequestedMute(bool mute) {
            if (mute) {
                _state |= RequestedMute;
            } else {
                _state &= ~RequestedMute;
            }
        }

        void setFill(bool fill) {
            if (fill) {
                _state |= Fill;
            } else {
                _state &= ~Fill;
            }
        }

        void setPattern(int pattern) {
            _pattern = pattern;
        }

        void setRequestedPattern(int pattern) {
            _requestedPattern = pattern;
        }

        uint16_t _state;
        uint8_t _pattern;
        uint8_t _requestedPattern;

        friend class PlayState;
        friend class Engine;
    };

    class SongState {
    public:
        bool playing() const { return _state & Playing; }
        bool hasPlayRequests() const { return hasRequests(PlayRequests); }
        int currentSlot() const { return _currentSlot; }
        int currentRepeat() const { return _currentRepeat; }
        int requestedSlot() const { return _requestedSlot; }

        void clear();

    private:
        enum State {
            Playing                 = 1<<0,

            ImmediatePlayRequest    = 1<<1,
            SyncedPlayRequest       = 1<<2,
            LatchedPlayRequest      = 1<<3,

            ImmediateStopRequest    = 1<<4,
            SyncedStopRequest       = 1<<5,
            LatchedStopRequest      = 1<<6,

            PlayRequests = ImmediatePlayRequest | SyncedPlayRequest | LatchedPlayRequest,
            StopRequests = ImmediateStopRequest | SyncedStopRequest | LatchedStopRequest,
            ImmediateRequests = ImmediatePlayRequest | ImmediateStopRequest,
            SyncedRequests = SyncedPlayRequest | SyncedStopRequest,
            LatchedRequests = LatchedPlayRequest | LatchedStopRequest
        };

        static State playRequestFromExecuteType(ExecuteType type) {
            return State(1<<(1 + type));
        }

        static State stopRequestFromExecuteType(ExecuteType type) {
            return State(1<<(4 + type));
        }

        void setRequests(int requests) {
            _state |= uint8_t(requests);
        }

        void clearRequests(int requests) {
            _state &= ~uint8_t(requests);
        }

        bool hasRequests(int requests) const {
            return _state & uint8_t(requests);
        }

        void setPlaying(bool playing) {
            if (playing) {
                _state |= Playing;
            } else {
                _state &= ~Playing;
            }
        }

        void setRequestedSlot(int slot) {
            _requestedSlot = slot;
        }

        void setCurrentSlot(int slot) {
            _currentSlot = slot;
        }

        void setCurrentRepeat(int slot) {
            _currentRepeat = slot;
        }

        uint8_t _state;
        uint8_t _requestedSlot;
        uint8_t _currentSlot;
        uint8_t _currentRepeat;

        friend class PlayState;
        friend class Engine;
    };

    //----------------------------------------
    // Properties
    //----------------------------------------

    // track states

    const TrackState &trackState(int track) const { return _trackStates[track]; }
          TrackState &trackState(int track)       { return _trackStates[track]; }

    // song state

    const SongState &songState() const { return _songState; }
          SongState &songState()       { return _songState; }

    //----------------------------------------
    // Methods
    //----------------------------------------

    PlayState(Project &project);

    // mutes

    void muteTrack(int track, ExecuteType executeType = Immediate);
    void unmuteTrack(int track, ExecuteType executeType = Immediate);
    void toggleMuteTrack(int track, ExecuteType executeType = Immediate);

    void muteAll(ExecuteType executeType = Immediate);
    void unmuteAll(ExecuteType executeType = Immediate);

    // solos

    void soloTrack(int track, ExecuteType executeType = Immediate);
    void unsoloTrack(int track, ExecuteType executeType = Immediate);

    // fills

    void fillTrack(int track, bool fill);
    void fillAll(bool fill);

    // pattern change

    void selectTrackPattern(int track, int pattern, ExecuteType executeType = Immediate);
    void selectPattern(int pattern, ExecuteType executeType = Immediate);

    // snapshots

    void createSnapshot();
    void revertSnapshot(int targetPattern = -1);
    void commitSnapshot(int targetPattern = -1);
    bool snapshotActive() const { return _snapshot.active; }

    // requests

    void cancelMuteRequests();
    void cancelPatternRequests();
    void commitLatchedRequests() { _executeLatchedRequests = true; }

    bool hasImmediateRequests() const { return _hasImmediateRequests; }
    bool hasSyncedRequests() const { return _hasSyncedRequests; }
    bool hasLatchedRequests() const { return _hasLatchedRequests; }

    // song

    void playSong(int slot, ExecuteType executeType = Immediate);
    void stopSong(ExecuteType executeType = Immediate);

    void clear();

    void write(WriteContext &context) const;
    void read(ReadContext &context);

private:
    void selectTrackPatternUnsafe(int track, int pattern, ExecuteType executeType = Immediate);

    void notify(ExecuteType executeType) {
        _hasImmediateRequests |= (executeType == Immediate);
        _hasSyncedRequests |= (executeType == Synced);
        _hasLatchedRequests |= (executeType == Latched);
    }

    bool executeLatchedRequests() const { return _executeLatchedRequests; }

    void clearImmediateRequests() { _hasImmediateRequests = false; }
    void clearSyncedRequests() { _hasSyncedRequests = false; }
    void clearLatchedRequests() { _hasLatchedRequests = false; _executeLatchedRequests = false; }

    Project &_project;

    std::array<TrackState, CONFIG_TRACK_COUNT> _trackStates;
    SongState _songState;

    bool _executeLatchedRequests;
    bool _hasImmediateRequests;
    bool _hasSyncedRequests;
    bool _hasLatchedRequests;

    static constexpr int SnapshotPatternIndex = CONFIG_PATTERN_COUNT;

    struct {
        bool active;
        uint8_t lastSelectedPatternIndex;
        uint8_t lastTrackPatternIndex[CONFIG_TRACK_COUNT];
    } _snapshot;

    friend class Project;
    friend class Engine;
};
