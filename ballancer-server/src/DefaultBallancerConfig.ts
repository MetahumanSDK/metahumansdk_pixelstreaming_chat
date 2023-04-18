import { IBallancerConfig } from "./IBallancerConfig";

export const DefaultBallancerConfig: IBallancerConfig = {
	hosting: {
		useHttps: false,
		port: 5000
	},
	ballancerServer: {
		port: 5001
	}
};
