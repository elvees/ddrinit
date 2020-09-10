node('linux') {
    stage('Checkout') {
        checkout scm
    }

    stage('Get version') {
        sh 'git describe --dirty --always > version.txt'
        version = readFile 'version.txt'
        currentBuild.displayName = "${version}"
    }

    env.PIPENV_SKIP_LOCK = 1
    env.SPHINXFLAGS = '-W'

    stage('Prepare environment') {
        sh '''
            python3.6 -m venv .venv
            source .venv/bin/activate
            pip install setuptools --upgrade
            pip install pip --upgrade
            pip install pipenv
            pipenv install --dev
        '''
    }

    stage('Build HTML') {
        sh '''
            source .venv/bin/activate
            make -C doc html
        '''

        publishHTML target: [
            reportName: 'ddrinit HTML documentation',
            reportTitles: 'ddrinit',
            reportDir: 'doc/build/html',
            reportFiles: 'index.html',
            allowMissing: false,
            alwaysLinkToLastBuild: false,
            keepAll: true
        ]
    }

    stage('Pre-commit') {
        sh '''
            source .venv/bin/activate
            pre-commit run -a --hook-stage manual
        '''
    }

    stage('Check') {
        sh '''
            source .venv/bin/activate
            make -C doc check
        '''
    }
}
