import * as express from 'express';
import * as HTTP from 'http';
import * as HTTPS from 'https';

import { getConfigFromArgvOrDefault, Logger } from '@pixstream/server';
import { toServerOptions } from '@pixstream/server/bin/connection';
import * as ClientMessages from '@pixstream/universal/bin/ClientMessages';
import * as StreamerMessages from '@pixstream/universal/bin/StreamerMessages';
import * as BallancerMessages from '@pixstream/server/bin/BallancerMessages';

import {
	BallancerCommunicator,
	BallancerCommunicatorStatus,
	StreamerCommunicator,
	StreamerCommunicatorStatus,
	PlayerCommunicator
} from '@pixstream/companion';

import { ICompanionServerConfig } from './ICompanionServerConfig';
import { DefaultCompanionServerConfig } from './DefaultCompanionServerConfig';

const logger = new Logger('Companion-Server');
const app = express();

const config: Readonly<ICompanionServerConfig> = getConfigFromArgvOrDefault(DefaultCompanionServerConfig);
logger.log("Config: " + JSON.stringify(config, null, '\t'));

const server = config.hosting.useHttps
	? new HTTPS.Server(toServerOptions(config.hosting), app)
	: new HTTP.Server(app)

// `clientConfig` is send to Streamer and Players
// Example of STUN server setting
const clientConfigMessage: Readonly<ClientMessages.IClientConfigMessage> = {
	type: 'config',
	AppName: 'DefaultClientConfigAppName',
	AppDescription: 'DefaultClientConfigAppDescription',
	peerConnectionOptions: {}
};

//Setup http and https servers
server.listen(config.hosting.port, () => {
	console.log(`${config.hosting.useHttps ? 'Https' : 'Http'} listening on *: ${config.hosting}`);
});

// Communcation with Unreal Engine

const streamerCommunicator = new StreamerCommunicator({
	hosting: config.streamer.hosting
});

streamerCommunicator.connect();

streamerCommunicator.onStatus(status => {
	if (status === StreamerCommunicatorStatus.Connected) {
		streamerCommunicator.sendMessage(clientConfigMessage);
	}
});

// Communication with browser

const playerCommunicator = new PlayerCommunicator({
	server: server
});

playerCommunicator.onMessage
	(
		(playerId, message) => {
			// todo: make this more flexible
			// if (SharedContracts.ToStreamerMessageTypesArray.includes(message.type)) {
			if (message.type === 'iceCandidate' || message.type === 'offer') {
				const combinedMessage: StreamerMessages.IToStreamerMessage = {
					...message,
					playerId: playerId,
					type: message.type
				};
				// todo:streamer coudld be not connected at this moment
				streamerCommunicator.sendMessage(combinedMessage);
				return;
			}

			logger.warn(`ignored message from player id = ${playerId} with type ${message.type}. ${JSON.stringify(message)}`);
		}
	);

playerCommunicator.onPlayerConnected
	(
		playerId => {
			if (streamerCommunicator.status !== StreamerCommunicatorStatus.Connected) {
				logger.error(`Player ${playerId} connected but streamer not ready. Disconnecting player.`);

				// Reject connection if streamer is not connected
				playerCommunicator.disconnectPlayerById(playerId, 1013 /* Try again later */, 'Streamer is not connected');
				return;
			}

			// send config
			playerCommunicator.sendMessageToPlayer(playerId, clientConfigMessage);
		}
	);

playerCommunicator.onPlayerDisconnected
	(
		(playerId, reason) => {
			const msg: StreamerMessages.IPlayerDisconnectedMessage = {
				type: 'playerDisconnected',
				reason: reason,
				playerId: playerId
			};

			streamerCommunicator.sendMessage(msg);
		}
	);

streamerCommunicator.onMessage
	(
		(playerId, msg) => {
			// player messages
			if (msg.type === 'disconnectPlayer') {
				const disconnectMessage = msg as StreamerMessages.IDisconnectPlayerMessage;
				playerCommunicator.disconnectPlayerById(playerId, 1011 /* internal error */, disconnectMessage.reason);
				return;
			}

			// todo: could be done less specific but this breaks types
			// if (SharedContracts.ToClientMessageTypesArray.includes(msg.type)) {
			if (msg.type === 'answer' || msg.type === 'iceCandidate') {
				const clientMessage: ClientMessages.IToClientMessage = {
					...msg,
					type: msg.type
				};
				playerCommunicator.sendMessageToPlayer(playerId, clientMessage);
				return;
			}

			logger.warn(`ignored message from streamer with type ${msg.type}. ${JSON.stringify(msg)}`);
		}
	);

// Communication with ballancer

if (config.ballancer) {
	const ballancer = new BallancerCommunicator({
		ballancer: config.ballancer
	});

	ballancer.connect();
	ballancer.registerMMKeepAlive();

	// bind ballancer connect events
	ballancer.onStatus((newStatus) => {
		if (newStatus !== BallancerCommunicatorStatus.Connected)
			return;

		const message: BallancerMessages.IConnectMessage = {
			type: 'connect',
			connection: config.hosting,
			ready: streamerCommunicator.status === StreamerCommunicatorStatus.Connected,
			playerCount: playerCommunicator.playerCount
		};

		ballancer.sendMessage(message);
	});

	// bind streamer events to ballancer
	streamerCommunicator.onStatus(newStreamerStatus => {
		if (newStreamerStatus === StreamerCommunicatorStatus.Connected) {

			if (streamerCommunicator.websocketState !== 1)
				logger.error(`${StreamerCommunicator.name} is connected but websocketState is not in ready state.`);

			const message: BallancerMessages.IStreamerConnectedMessage = {
				type: 'streamerConnected'
			};

			ballancer.sendMessage(message);
			return;
		}

		if (newStreamerStatus === StreamerCommunicatorStatus.Disconnected) {
			const message: BallancerMessages.IStreamerDisconnectedMessage = {
				type: 'streamerDisconnected'
			};
			ballancer.sendMessage(message);
			playerCommunicator.disconnectAllPlayers(undefined, 'streamer is desconnected');
			return;
		}
	});

	// bind to player communicator
	playerCommunicator.onPlayerConnected
		(
			playerId => {
				// The ballancer will not re-direct clients to this Companion server if any client
				// is connected.
				const message: BallancerMessages.IClientConnectedMessage = {
					type: 'clientConnected',
					playerCount: playerCommunicator.playerCount,
					playerId
				};

				ballancer.sendMessage(message);
			}
		);

	playerCommunicator.onPlayerDisconnected
		(
			playerId => {
				const message: BallancerMessages.IClientDisconectedMessage = {
					type: 'clientDisconnected',
					playerCount: playerCommunicator.playerCount,
					playerId
				};

				ballancer.sendMessage(message);
			}
		);
}
