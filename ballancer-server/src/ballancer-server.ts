import * as express from 'express';
import * as HTTP from 'http';
import * as HTTPS from 'https'
import * as cors from 'cors';

import * as Connection from "@pixstream/server/bin/connection";
import { getConfigFromArgvOrDefault } from '@pixstream/server';

import { ICompanionConnectionSettings } from '@pixstream/universal';
import { BallancerServer } from '@pixstream/ballancer';
import { DefaultBallancerConfig } from './DefaultBallancerConfig';
import { IBallancerConfig } from './IBallancerConfig';

export function appFactory(config: IBallancerConfig): express.Express {
  const server = express();

  const ballancer = new BallancerServer({
    port: config.ballancerServer.port
  });

  server.options('/api/companion', cors());
  server.get('/api/companion', cors(), (req, res) => {
    const companionServer = ballancer.getAvailableCompanionServer();

    if (companionServer === undefined || companionServer === null) {
      res.json({ error: 'No companion servers available' });
      return;
    }

    const connectionStrings: ICompanionConnectionSettings = {
      websocketUrl: Connection.toWebsocketUrl(companionServer.connection),
      serverUrl: Connection.toHttpUrl(companionServer.connection)
    }

    res.json(connectionStrings);
  });

  ballancer.start();

  return server;
}

function run(): void {
  const config = getConfigFromArgvOrDefault(DefaultBallancerConfig);
  const app = appFactory(config);

  const server = config.hosting.useHttps
    ? new HTTPS.Server(Connection.toServerOptions(config.hosting), app)
    : new HTTP.Server(app)

  server.listen(config.hosting.port, () => {
    console.log(`HTTP listening on *:${config.hosting.port}`);
  });
}

// Webpack will replace 'require' with '__webpack_require__'
// '__non_webpack_require__' is a proxy to Node 'require'
// The below code is to ensure that the server is run only when not requiring the bundle.
declare const __non_webpack_require__: NodeRequire;
const mainModule = __non_webpack_require__.main;
const moduleFilename = mainModule && mainModule.filename || '';

if (moduleFilename === __filename || moduleFilename.includes('iisnode')) {
  run();
}