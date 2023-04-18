import { ICompanionServerConfig } from './ICompanionServerConfig';

export const DefaultCompanionServerConfig: ICompanionServerConfig = {
    hosting: {
        useHttps: false,
        host: 'localhost',
        port: 5002
    },
    streamer: {
        hosting: {
            useHttps: false,
            host: 'localhost',
            port: 8888
        }
    },
    ballancer: {
        connection: {
            host: 'localhost',
            port: 5001
        },
        retryInterval: 5 * 1000,
        keepAliveInterval: 30 * 1000
    },
    peerConnectionOptions: {
        iceServers: [
            {
                urls: [
                    "stun:stun1.l.google.com:19302",
                    "turn:127.0.0.1:19303"
                ],
                username: "PixelStreamingUser",
                credential: "AnotherTURNintheroad"
            }
        ]
    }
};
