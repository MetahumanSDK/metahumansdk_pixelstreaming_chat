const path = require('path');

module.exports = {
    entry: './src/companion-server.ts',
    module: {
        rules: [
            {
                test: /\.tsx?$/,
                loader: 'ts-loader',
                exclude: /node_modules/,
                options: {
                    configFile: 'tsconfig.json'
                }
            },
        ],
    },
    target: 'node',
    resolve: {
        extensions: ['.ts', '.js'],
    },
    output: {
        filename: 'companion-server-bundled.js',
        library: {
            name: 'NodeBundle',
            type: 'umd',
        },
        globalObject: 'this',
        path: path.resolve(__dirname),
    },
    optimization: {
        minimize: false,
    }
};

