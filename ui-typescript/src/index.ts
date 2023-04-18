import './index.scss';

import { ICompanionConnectionSettings } from '@pixstream/universal';
import { AfkStatus, CompanionCommunicatorStatus, Streamer, IAfkEvent } from '@pixstream/browser';

import { Assets } from './Assets';

import { getRequiredElementById, getExactElementById, getInputById, toggleClass, toggleVisibility, renderVariableValue } from './utils';

import { ShowcaseStatus } from './ShowcaseStatus';
import { MyUnrealDescriptorTypes } from './MyDescriptors';

const _window: any = typeof window === 'undefined' ? {} : window;

const settings = {
    ballancerHost: 'http://localhost:5000'
}

const streamingContainer = getExactElementById('streamingContainer', HTMLDivElement);
const streamingVideo = getExactElementById('video', HTMLVideoElement);

const streamer = new Streamer<MyUnrealDescriptorTypes>({
    videoContainer: streamingContainer,
    video: streamingVideo
});

const audio = getExactElementById('audio', HTMLAudioElement);
const phraseFormControl = getInputById('phraseFormControl');

const debugMessages = getRequiredElementById('debugMessages');
const statusText = getRequiredElementById('statusText');

// state popups
const startPopup = getRequiredElementById('startPopup'),
    afkWarningPopup = getRequiredElementById('afkWarningPopup'),
    retryPopup = getRequiredElementById('retryPopup'),
    innactivityPopup = getRequiredElementById('innactivityPopup'),
    reconnectPopup = getRequiredElementById('reconnectPopup');

let status = ShowcaseStatus.NotStarted
let companionStatus = CompanionCommunicatorStatus.NotSet;

let afkStatus = AfkStatus.Stopped;

let serverUrl = '',
    errorText = '',
    bgVideoUrl = '',
    bgVideoPosterUrl = '';

let reconnectionTime = 0; // in seconds
let reconnectionIntervalId: ReturnType<typeof setInterval> | undefined;

let remainingTime = 0;// in seconds
let remainingIntervalId: ReturnType<typeof setInterval> | undefined;

// update all properties
function updateUI() {
    toggleClass(streamingContainer, 'not-ready', companionStatus !== CompanionCommunicatorStatus.StreamingInProgress);

    toggleVisibility(startPopup, status === ShowcaseStatus.NotStarted);
    toggleVisibility(afkWarningPopup, afkStatus === AfkStatus.Warning && status === ShowcaseStatus.Streaming);
    toggleVisibility(retryPopup, status === ShowcaseStatus.Retrying);
    toggleVisibility(innactivityPopup, afkStatus === AfkStatus.Finished && status === ShowcaseStatus.WaitUserReconnect);
    toggleVisibility(reconnectPopup, afkStatus !== AfkStatus.Finished && status === ShowcaseStatus.WaitUserReconnect);

    debugMessages.innerHTML = `
    status: ${status}<br/>
    companionStatus: ${companionStatus}<br/>
    serverUrl: ${serverUrl}
    `;

    toggleVisibility(statusText, status === ShowcaseStatus.Connecting);

    renderVariableValue('errorText', errorText);
    renderVariableValue('remainingTime', remainingTime);
    renderVariableValue('reconnectionTime', reconnectionTime);
    renderVariableValue('companionStatus', companionStatus);
}

async function start(): Promise<void> {
    await audio.play();
    await reconnect();

    updateUI();;
}

async function onStreamerStatus(newStatus: CompanionCommunicatorStatus): Promise<void> {
    companionStatus = newStatus;
    updateUI();;

    console.log(`Copmanion status : ${companionStatus}`);

    if (newStatus === CompanionCommunicatorStatus.StreamingReadyToStart) {
        console.log('ready to start!');

        await streamer.play();

        streamer.communicator.emitUIInteraction({
            type: 'ready',
            message: ''
        });

        // streamer often breaks here so I added delay
        setTimeout(() => {
            console.log(`requestQualityControl!`);
            streamer.communicator.requestQualityControl();
        }, 250);

        setTimeout(() => {
            phraseFormControl.disabled = false;

            status = ShowcaseStatus.Streaming;
            updateUI();;

        }, 500);
    }

    if (newStatus === CompanionCommunicatorStatus.Closed) {
        status = ShowcaseStatus.WaitUserReconnect;
    }

    updateUI();;
}

function onAfkWarning(event: IAfkEvent): void {
    console.log(`AFK WARNING status ${event.status}. remaining time ${streamer.afkRemainingTime}`);
    afkStatus = event.status;

    if (event.status === AfkStatus.Stopped || event.status === AfkStatus.Finished) {
        clearInterval(remainingIntervalId);
        remainingTime = 0;
        return;
    }

    startRemainingTimer();
}

function sendForm(): void {
    if (phraseFormControl.value.trim().length === 0)
        return;

    const text = phraseFormControl.value;

    phraseFormControl.value = '';

    sendChatMessage(text);

    if (status === ShowcaseStatus.NotStarted) {
        phraseFormControl.disabled = true;

        return;
    }

    updateUI();;
}

function sendChatMessage(text: string, event?: Event) {
    streamer.communicator.emitUIInteraction({ type: 'chat', message: text });

    updateUI();
}

async function reconnect(): Promise<void> {
    status = ShowcaseStatus.Connecting;
    errorText = '';

    updateUI();;


    let data: ICompanionConnectionSettings;

    try {
        const response = await fetch(`${settings.ballancerHost}/api/companion`);

        if (!response.ok) {
            const body = await response.text();
            alert(`Connection Error ${response.status}. ${response.statusText}. body: ${body}`);
            return;
        }

        data = await response.json();
    }

    catch (e) {
        alert(`Connection Error ${e}`);
        return;
    }

    errorText = data?.error ?? '';

    if (errorText.length > 0) {

        status = ShowcaseStatus.Retrying;
        startReconnectionTimer();

        updateUI();;
        return;
    }

    serverUrl = data.serverUrl;

    streamer.start(data.websocketUrl);

    updateUI();;
}

async function startIfNotStarted(): Promise<void> {
    if (status === ShowcaseStatus.NotStarted)
        await start();
}

function startReconnectionTimer(): void {
    reconnectionTime = 10;

    clearInterval(reconnectionIntervalId);

    reconnectionIntervalId = setInterval(() => {

        reconnectionTime--;

        if (reconnectionTime > 0) {
            updateUI();;
            return;
        }

        clearInterval(reconnectionIntervalId);
        reconnectionTime = 0;
        reconnect();
        updateUI();;

    }, 1000);

    updateUI();;
}

function startRemainingTimer(): void {
    remainingTime = Math.round(streamer.afkRemainingTime / 1000);

    clearInterval(remainingIntervalId);

    remainingIntervalId = setInterval(() => {
        remainingTime = Math.round(streamer.afkRemainingTime / 1000);

        if (remainingTime > 0) {
            updateUI();;
            return;
        }

        clearInterval(remainingIntervalId);

        remainingTime = 0;
        updateUI();
    }, 500);

    updateUI();;
}

function init(): void {
    streamer.$status.subscribe(s => onStreamerStatus(s));
    streamer.$afkWarning.subscribe(e => onAfkWarning(e));

    audio.src = Assets.emptySoundUrl;

    // Images
    getExactElementById('sendIconImage', HTMLImageElement).src = Assets.sendIconUrl;

    // bind functions so they can be called from html
    _window.startIfNotStarted = startIfNotStarted;
    _window.sendForm = sendForm;
    _window.reconnect = reconnect;

    updateUI();;
}

init();