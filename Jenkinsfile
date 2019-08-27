pipeline {
    agent none

    environment {
        CONAN_USER_HOME_SHORT = 'None'
        OSP_CONAN_CREDS = credentials('jenkins-osp-conan-creds')
    }

    stages {

        stage('Build') {
            parallel {
                stage('Build on Windows') {
                    agent { label 'windows' }

                    environment {
                        CONAN_USER_HOME = "${env.SLAVE_HOME}/conan-repositories/${env.EXECUTOR_NUMBER}"
                    }
                    
                    stages {
                        stage('Configure Conan') {
                            steps {
                                sh 'conan remote add osp https://osp-conan.azurewebsites.net/artifactory/api/conan/conan-local --force'
                                sh 'conan user -p $OSP_CONAN_CREDS_PSW -r osp $OSP_CONAN_CREDS_USR'
                            }
                        }
                        stage('Build Debug') {
                            steps {
                                dir('debug-build') {
                                    bat 'conan install ../ -s build_type=Debug -b missing'
                                    bat 'cmake -G "Visual Studio 15 2017 Win64" ../'
                                    bat 'cmake --build . --config Debug'
                                }
                            }
                        }
                        stage('Build Release') {
                            steps {
                                dir('release-build') {
                                    bat 'conan install ../ -s build_type=Release -b missing'
                                    bat 'cmake -G "Visual Studio 15 2017 Win64" ../'
                                    bat 'cmake --build . --config Release'
                                }
                            }
                            post {
                                success {
                                    dir('release-build/Release') {
                                        archiveArtifacts artifacts: '**',  fingerprint: true
                                    }
                                }
                            }
                        }
                    }
                }
                stage ( 'Build on Linux' ) {
                    agent {
                        dockerfile {
                            filename 'Dockerfile'
                            dir '.dockerfiles'
                            label 'linux && docker'
                            args '-v ${SLAVE_HOME}/conan-repositories/${EXECUTOR_NUMBER}:/conan_repo'
                        }
                    }

                    environment {
                        CONAN_USER_HOME = '/conan_repo'
                    }

                    stages {
                        stage('Configure Conan') {
                            steps {
                                sh 'conan remote add osp https://osp-conan.azurewebsites.net/artifactory/api/conan/conan-local --force'
                                sh 'conan user -p $OSP_CONAN_CREDS_PSW -r osp $OSP_CONAN_CREDS_USR'
                            }
                        }
                        stage('Build Debug') {
                            steps {
                                dir('debug-build') {
                                    sh 'conan install ../ -s compiler.libcxx=libstdc++11 -s build_type=Debug -b missing'
                                    sh 'cmake -DCMAKE_BUILD_TYPE=Debug ../'
                                    sh 'cmake --build .'
                                }
                            }
                        }
                        stage('Build Release') {
                            steps {
                                dir('release-build') {
                                    sh 'conan install ../ -s compiler.libcxx=libstdc++11 -s build_type=Release -b missing'
                                    sh 'cmake -DCMAKE_BUILD_TYPE=Release ../'
                                    sh 'cmake --build .'
                                }
                            }
                            post {
                                success {
                                    dir('release-build') {
                                        archiveArtifacts artifacts: 'cse',  fingerprint: true
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
