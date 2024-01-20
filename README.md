# victo-rdb
Victo: Realtime AI Native Database

# Introduction
Welcome to Victo project.

Victo is a realtime AI Native Database for effeciently storing and retriving vectors. Victo is a database server leveraging the power of websockets which means to connect to Victo all that we need is a websocket client. Connect seamlessly with application written in any language, unlocking the potential for realtime communication and efficient vector operations.

We hope you enjoy using Victo.

# Get Started

<div align="center"><h4>The current version of Victo works only in Unix and Unix-like OS</h4></div>

## Prerequisites
1. GCC
2. CMake (VERSION 3.27.1)

## Build

1. Fork the repo
2. Clone your forked repo
3. CMake is used to manage the build process. In the cloned repo, navigate to the root directory
```
cd path/to/cloned/repo
```
Replace "path/to/cloned/repo" with actual path to your cloned repo
4. Build the executable using following commands
```
cmake -S . -B ./build
make -C ./build/
```
5. Once the step 4 is successful, "victo-exe" would have been created in "path/to/cloned/repo/<b>build</b>" directory
6. Navigate to "build" directory and start Victo database using below command
```
./build/victo-exe -d /preferred/path/for/victo/data/files
```
It is in this path the actual victo database objects and files are stored in the disk. Other supported flags including -d are:

| Flag     | Description                                          | Mandatory?        | Default Value        | Valid values  |
|----------|------------------------------------------------------|-------------------|----------------------|---------------|
|   -d    | Preferred path to victo database                      | Yes               |                      | File Path     |
|   -i    | overwrite ip (useful while starting in container)     | No                | 127.0.0.1 or 0.0.0.0 | IP Address    |
|   -p    | overwrite port                                        | No                | 2018                 | Port Number   |
|   -l    | Logger Level                                          | No                | INFO                 | DEBUG         |
|         |                                                       |                   |                      | INFO          |
|         |                                                       |                   |                      | WARN          |
|         |                                                       |                   |                      | ERROR         |
|         |                                                       |                   |                      | CRITICAL      |


7. By default, the db server is started listening on port 2018. We can connect to the server with any websocket client using the URL as below
```
ws://<host-ip>:2018/
```

## Docker Support

Alternatively, you can run Victo as a Docker container. 

### How to start Victo as a Docker Container?

Starting a Victo Server Instance as a Docker container is simple
```
docker run --name some-name -p <host-port>:8080 -v <host-path>:/app/victodb  -d sreehari006/victo:tag
```
... where "some-name" is the name you want to provide to the container. Inside the container Victo runs on port 8080. <host-port> is the port on host machine that you want to map to the container port. <host-path> is the path on host machine where the volume data is stored. "tag" is the version of Victo that you want to run.

For more detail, please visit [https://hub.docker.com/repository/docker/sreehari006/victo/general](https://hub.docker.com/repository/docker/sreehari006/victo/general)

# Technologies/Concepts
1. AI and Vector analysis
2. WebSockets and Socket programming
3. JSON parser
4. Abstract Syntax Tree
5. UUID generator

# How it works
The fundamental components of Victo Database are:
* <b>db: </b>The basic components which holds a list of collections. Victo databse server can have multiple database objects.
* <b>collection: </b>Collection holds a list of vectors
* <b>vector: </b>Vectors are data points or entities which represent vector embeddings
* <b>queries: </b>Queries are JSON-like string used to interact with database for storing and retriving db objects. 

## DB Operations and Syntax
The following list of DB operations can be executed on the server using any websocket client

### Basic structure of a query
As mentioned earlier, query is a JSON-like string. Any victo query has 3 basic components

| Key     | Description                                           | Valid values                                                  |
|---------|-------------------------------------------------------|---------------------------------------------------------------|
| op      | operation on the Database                             | "add" if the obj is db                                        |
|         |                                                       | "add", "delete", "list" or "count" if obj is collection       |
|         |                                                       | "put", "get", "list", "count" or "query" if obj is vector     |
| obj     | Database object on which the operation is executed    | "db" or "collection" or "vector"                              |
| args    | arguments                                             |                                                               |

### Add a database 

#### Request
```
{   
    "op":"add", 
    "obj":"db", 
    "args": {
        "db": <preferred-name>
    }
}
```

| Key     | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|---------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db      | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

### Add a collection 

#### Request
```
{   
    "op":"add", 
    "obj":"collection", 
    "args": {
        "db": <db-name>,
        "collection": <preferred-name>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

### Delete a collection 

#### Request
```
{   
    "op":"delete", 
    "obj":"collection", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

### List total number of collections in a database 

#### Request
```
{   
    "op":"count", 
    "obj":"collection", 
    "args": {
        "db": <db-name>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "count": <integer>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```


### List all collections in a database 

#### Request
```
{   
    "op":"list", 
    "obj":"collection", 
    "args": {
        "db": <db-name>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "collections: [
                "array-of-collections-uuid"
            ]
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

### Add a vector to a collection 

#### Request
```
{   
    "op":"put", 
    "obj":"vector", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>,
        "ai_model": <ai-model-with-which-vector-embeddings-are-generated>,
        "vdim": <vector-dimension">,
        "vp": <array-of-vector-datapoints>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| ai_model    | AI Model              | Yes               | String        |                                                 |
| vdim        | Vector Dimension      | Yes               | Integer       |                                                 |
| vp          | Vector Points         | Yes               | Decimal Array |                                                 |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "hash": <uuid>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

### Retrive a vector to a collection 

#### Request
```
{   
    "op":"get", 
    "obj":"vector", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>,
        "hash": <vector-id>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| hash        | Vector ID             | Yes               | UUID          |                                                 |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "hash": <uuid>,
            "ai_model": <ai-model>,
            "dimension": <integer>,
            "is_normal": <boolean-true-if-normalized-else-false>,
            "vp": <array-of-decimal-datapoints>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

### Delete a vector in a collection 

#### Request
```
{   
    "op":"get", 
    "obj":"vector", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>,
        "hash": <vector-id>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| hash        | Vector ID             | Yes               | UUID          |                                                 |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

### List total number of vectors in a collection 

#### Request
```
{   
    "op":"count", 
    "obj":"vector", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "count": <integer>
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

### List all vectors in a collection 

#### Request
```
{   
    "op":"list", 
    "obj":"vector", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>
    }
}
```

| Key         | Description           | Is Mandatory?     | Data Type     | Valid values                                    |
|-------------|-----------------------|-------------------|---------------|-------------------------------------------------|
| db          | DB Name               | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection  | Collection Name       | Yes               | String        | Alphanumeric value with hyphen and underscore   |

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "vectors": [
                <array-of-vector-uuids>
            ]
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

### Query a vector 

#### Request
```
{   
    "op":"query", 
    "obj":"vector", 
    "args": {
        "db": <db-name>,
        "collection": <collection-name>,
        "ai_model": <ai-model-with-which-vector-embeddings-are-generated>,
        "vdim": <vector-dimension">,
        "vp": <array-of-vector-datapoints>
        "qOps": {
            "vd_method": <preferred-vector-distance-calculation-method>,
            "k_value": <base-value-for-finding-nearest-vectors>,
            "logical_op": <logic-operation-in-query-comparison-against-kvalue>,
            "limit": <number-of-objects-returned-in-query>,
            "order": <order-in-which-query-is-returned>,
            "include_fault": <include-fault-nodes>,
            "do_normal": <normalize-before-query-comparison>,
            "p_value": <applicable-only-for-minskowski-method>
        }
    }
}
```

| Key             | Description                                       | Is Mandatory?     | Data Type     | Valid values                                    |
|-----------------|-----------------------|-------------------|---------------|-----------------------------------------------------------------------------|
| db              | DB Name                                           | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| collection      | Collection Name                                   | Yes               | String        | Alphanumeric value with hyphen and underscore   |
| ai_model        | AI Model                                          | Yes               | String        |                                                 |
| vdim            | Vector Dimension                                  | Yes               | Integer       |                                                 |
| vp              | Vector Points                                     | Yes               | Decimal Array |                                                 |
| vd_method       | Vector Distance Calculation Method                | No                | Integer       | Default value is 0                              |
|                                                                                                         | 0 - EUCLIDEAN_DISTANCE                          |
|                                                                                                         | 1 - COSINE_SIMILARITY                           |
|                                                                                                         | 2 - MANHATTAN_DISTANCE                          |
|                                                                                                         | 3 - MINKOWSKI_DISTANCE                          |
|                                                                                                         | 4 - DOT_PRODUCT                                 |
| k_value         | base value used for query on calculated distance  | No                | Decimal       | Default 0.0                                     |
| logical_op      | logical operation against k_value while query     | No                | Integer       | Default 0                                       |
|                                                                                                         | 2  - GREATER_THAN_OR_EQUAL_TO                   |
|                                                                                                         | 1  - GREATER_THAN                               |
|                                                                                                         | 0  - EQUAL_TO                                   |
|                                                                                                         | -1 - LESS_THAN                                  |
|                                                                                                         | -2 - LESS_THAN_OR_EQUAL_TO                      |
| limit           | Number of vector points returned                  | No                | Integer       | Positive Integer                                |
| order           | Ascending or descending based on k_value          | No                | Boolean       | Default is false                                |
|                                                                                                         | false - Ascending                               |
|                                                                                                         | true - Descending                               |
| include_fault   | Include fault nodes                               | No                | Boolean       | Default is false                                |
|                                                                                                         | false - do not include fault nodes              |
|                                                                                                         | true - include fault nodes                      |
| do_normal       | Normalize vector points while query               | No                | Boolean       | Default is false                                |
|                                                                                                         | false - do not normalize                        |
|                                                                                                         | true - normalize vector                         |
| p_value         | Applicable only for Minskowski Method             | Conditional       | Decimal       |                                                 |

<i>Fault nodes are something where the query vector and vectors are incompatible. For example, if the query vector dimension is not same as vector data points dimension.</i>

#### Response
```
{
    "metadata": [
        {
            "response_id": <response-id>
        }
    ],
    "result": [
        {
            "code": <interger-result-code>,
            "message": <result-message>,
            "vectors": [
                {
                    "code": <interger-result-code>,
                    "message": <result-message>,
                    "ai_model": <ai-model>,
                    "hash": <vector-uuid>,
                    "is_normal": <Y-if-normalized-else-N>,
                    "dimension": <integer-vector-dimension>,
                    "distance": <distance-from-k_value>
                }
            ],
            "faults": [
                {
                    "code": <interger-result-code>,
                    "message": <result-message>,
                    "ai_model": <ai-model>,
                    "hash": <vector-uuid>,
                    "is_normal": <Y-if-normalized-else-N>,
                    "dimension": <integer-vector-dimension>,
                    "distance": 0
                }

            ]
        }
    ],
    "error": [
        "array-of-error-strings-if-any
    ]
}
```

# Practical usecases
* NLP
* Generative AI
* Recommender System
* Viedo Search
* Image Search
* eCommerce
* Machine Learning
* Social Networks

## Roadmap
- [x] Enhance logging mechanism and Error Handling
- [ ] Build scheduler for threads handling db operations
- [ ] Enable support for secure websockets (wss)
- [ ] Enhance DB features
    - [ ] Users accounts and access control
    - [ ] Subscribe on collections

## Version
[20 Jan 2024] - 1.2.0 (Latest stable version)
Enhanced logging mechanism and error handling

[12 Jan 2024] - 1.1.0
Added support for Docker and containerization

[6th Jan 2024] - 1.0.0 

## License

Distributed under the MIT License. See `LICENSE.txt` for more information.

# Contributing

Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

# Contact

Sree Hari 
eMail - hari.tinyblitz@gmail.com
LinkedIn - [view_my_profile] (https://www.linkedin.com/in/sree-hari-84911b123/)

# Acknowledgements

1. [Best-README-Template](https://github.com/othneildrew/Best-README-Template)
