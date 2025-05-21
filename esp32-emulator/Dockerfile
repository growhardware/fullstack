FROM node:18-alpine
WORKDIR /app
COPY . .
RUN npm install
EXPOSE 8000
# EXPOSE 8082
CMD ["node", "index.js"]