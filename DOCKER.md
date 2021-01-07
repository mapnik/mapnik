# Using Mapnik with Docker

Clone the repository with 
```bash
git clone https://github.com/mapnik/mapnik.git
cd mapnik
```

Build the Docker image with
```bash
docker build -t mapnik .
```

Extend the image with a custom `Dockerfile` like
```dockerfile
FROM mapnik

# other dependencies

ENTRYPOINT mapnik-script
```