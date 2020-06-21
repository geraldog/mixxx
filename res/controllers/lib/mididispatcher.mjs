export class MidiDispatcher {
    constructor(noteOff) {
        this.noteOff = noteOff;
        this.inputMap = new Map();
    }
    setInputCallback(midiBytes, callback) {
        if (!Array.isArray(midiBytes)) {
            throw new Error('MidiDispatcher.setInputCallback midiBytes must be an Array, received ' + midiBytes);
        }
        if (typeof midiBytes[0] !== 'number') {
            throw new Error('MidiDispatcher.setInputCallback midiBytes must be an Array of numbers, received ' + midiBytes);
        }
        if (typeof callback !== 'function') {
            throw new Error('MidiDispatcher.setInputCallback callback must be a function, received ' + callback);
        }
        // JavaScript is broken and believes [1,2] === [1,2] is false, so
        // JSONify the Array to make it usable as a Map key.
        const key = JSON.stringify(midiBytes);
        this.inputMap.set(key, callback);
        // If passed a Note On message, also map the corresponding Note Off
        // message to the same callback.
        if (this.noteOff === true && ((midiBytes[0] & 0xF0) === 0x90)) {
            const noteOffBytes = [midiBytes[0] - 0x10, midiBytes[1]];
            const noteOffKey = JSON.stringify(noteOffBytes);
            this.inputMap.set(noteOffKey, callback);
        }
    }
    receiveData(data, timestamp) {
        let key;
        // MIDI messages starting with 0xC (program change) or 0xD (aftertouch) messages are only
        // two bytes long and distinguished by their first byte.
        // https://www.midi.org/specifications-old/item/table-2-expanded-messages-list-status-bytes
        if ((data[0] & 0xF0) == 0xC0 || (data[0] & 0xF0) === 0xD0) {
            key = JSON.stringify([data[0]]);
            print('SINGLE ' + data);
        } else {
            key = JSON.stringify([data[0], data[1]]);
        }

        const callback = this.inputMap.get(key);
        if (typeof callback === 'function') {
            callback(data, timestamp);
        }
    }
}
