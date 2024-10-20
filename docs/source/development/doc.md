# How to contibute to Documentation

The CEPCSW user guide is created using a tool called sphinx. Sphinx could parse the Markdown files and then create HTML files. The Markdown files are all maintained in the source code of CEPCSW.

Here is an instruction to add or modify the user guide:
* Fork the CEPCSW and git clone the source code
```bash
$ git clone git@code.ihep.ac.cn:cepc/CEPCSW.git
$ cd CEPCSW/docs/
```
* Add or modify the markdown files
  * If you need to add new pages, please add file names in the index.rst. 
* If sphinx is not available, you need to install it and its dependencies. See the below instructions.
```bash
$ python -m venv venv             # create a virtual environment
$ source venv/bin/activate        # activate the venv
$ pip install -r requirements.txt # install the dependencies
```
* Build the user guide with sphinx
```bash
$ make html
```
* Run a lightweight web server using python. 
  * The URL will consists of two parts: server name and port number. 
  * Use the command `hostname` to get the name of the server name. If you build the docs in your local computer, just use `localhost`.
  * If the port number `8888` is already used by others, just change to some other number. 
  
```bash
$ python -m http.server 8888 --bind 0.0.0.0 --directory build/html
```
* Use Web Browser to view the document. 
  * For example: http://localhost:8888
* If there is no issue, then `git commit` and `git push`. Create a MR when it is ready.  
