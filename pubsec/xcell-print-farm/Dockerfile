# Build stage
FROM node:20-alpine AS builder
WORKDIR /app

# Copy package files
COPY package*.json ./

# Install dependencies
RUN npm ci

# Copy source
COPY . .

# Build arguments for Ditto config (passed at build time)
ARG DITTO_APP_ID
ARG DITTO_PLAYGROUND_TOKEN
ARG DITTO_AUTH_URL
ARG DITTO_WEBSOCKET_URL

# Create env file for build
RUN echo "DITTO_APP_ID=${DITTO_APP_ID}" > .env.local && \
    echo "DITTO_PLAYGROUND_TOKEN=${DITTO_PLAYGROUND_TOKEN}" >> .env.local && \
    echo "DITTO_AUTH_URL=${DITTO_AUTH_URL}" >> .env.local && \
    echo "DITTO_WEBSOCKET_URL=${DITTO_WEBSOCKET_URL}" >> .env.local

# Build the app
RUN npm run build

# Production stage
FROM nginx:alpine
WORKDIR /usr/share/nginx/html

# Copy built assets
COPY --from=builder /app/dist .

# Copy nginx config
COPY nginx.conf /etc/nginx/conf.d/default.conf

EXPOSE 80

CMD ["nginx", "-g", "daemon off;"]
