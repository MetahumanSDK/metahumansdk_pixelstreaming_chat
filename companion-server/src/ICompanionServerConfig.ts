import * as Connection from "@pixstream/server/bin/connection";
import { IIceServer } from "@pixstream/companion";

export interface ICompanionServerConfig {
    hosting: Connection.ExtendedHttpHostSettings | Connection.ExtendedHttpsHostSettings;

    ballancer?: {
        connection: Connection.ConnectionSettings;
        retryInterval: number;
        keepAliveInterval: number
    }

    streamer: {
        hosting: Connection.ExtendedHttpHostSettings | Connection.ExtendedHttpsHostSettings;
    }

    peerConnectionOptions: {
        iceServers: IIceServer[];
    }
}
