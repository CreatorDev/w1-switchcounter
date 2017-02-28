#!/usr/bin/env groovy

/**
 * Build Switchcounter Workshop
 *
 */

properties([
    buildDiscarder(logRotator(numToKeepStr: '30')),
    parameters([
        stringParam(defaultValue: 'http://downloads.creatordev.io/creator-openwrt/v1/pistachio/marduk/creator-sdk-1.tar.bz2',
            description: 'Creator image tarball to use', name: "CREATOR_IMAGE"),
    ])
])

node('docker && imgtec') {
    def docker_image
    docker_image = docker.image "imgtec/creator-builder:latest"
    docker_image.inside {
        stage('Prepare') {
            checkout([$class: 'GitSCM',
                userRemoteConfigs: scm.userRemoteConfigs,
                branches: scm.branches,
                doGenerateSubmoduleConfigurations: scm.doGenerateSubmoduleConfigurations,
                submoduleCfg: scm.submoduleCfg,
                browser: scm.browser,
                gitTool: scm.gitTool,
                extensions: scm.extensions + [
                    [$class: 'CleanCheckout'],
                    [$class: 'PruneStaleBranch'],
                    [$class: "RelativeTargetDirectory", relativeTargetDir: "${WORKSPACE}/workshop/switch-counter"],
                ],
            ])
            sh "wget -qO- ${params.CREATOR_IMAGE} | tar -xj --strip-components 2"
            sh "echo 'src-link workshop ../workshop' >> feeds.conf.default"
            sh "cat Config.in feeds.conf.default"
            sh "scripts/feeds update -a && scripts/feeds install -a"
        }
        stage('Build') {
          sh "make -j1 V=s package/switch-counter/compile"
        }
        stage('Upload') {
            archiveArtifacts 'bin/pistachio/packages/workshop/*'
            deleteDir()  // clean up the workspace to save space
        }
    }
}
