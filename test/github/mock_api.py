#
#

from fastapi import FastAPI
from fastapi import status

app = FastAPI()


@app.get("/repos/owner/repo")
async def repo_works():
    return {"default_branch": "default_branch"}


@app.get("/repos/owner/repo_fail")
async def repo_fails():
    return {}


@app.get("/repos/owner/repo_invalid_response")
async def repo_invalid_response():
    return "NOT JSON"


@app.get("/ping", status_code=status.HTTP_200_OK, description="Health check endpoint")
def ping():
    """Return health check status."""
    return {"status": "ok"}
