node('linter') {
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
        sh 'pipenv install --dev'
    }

    stage('Build HTML') {
        sh 'pipenv run make -C doc html'

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
        sh 'pre-commit run -a --hook-stage manual'
    }

    stage('Check') {
        sh 'pipenv run make -C doc check'
    }

    stage('Build') {
        sh '''
            module load cmake
            module load toolchain/mips/codescape/img/bare/2018.09-03
            pipenv run make solarisbub_defconfig
            pipenv run make CROSS_COMPILE=mips-img-elf-
        '''
    }
}
