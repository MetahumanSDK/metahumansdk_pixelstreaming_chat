import * as Connection from "@pixstream/server/bin/connection";

export interface IBallancerConfig {
    hosting: Connection.HttpHostSettings | Connection.HttpsHostSettings;
    ballancerServer: {
        port: number
    }
}
